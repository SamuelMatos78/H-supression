#define _POSIX_C_SOURCE 200809L
#include "agent.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
typedef union {max_align_t alignment;size_t bytes;} Allocation;
double ag_now(void){struct timespec t;if(clock_gettime(CLOCK_MONOTONIC,&t))abort();return t.tv_sec*1e9+t.tv_nsec;}
uint64_t ag_random(uint64_t *rng){uint64_t z=(*rng+=UINT64_C(0x9e3779b97f4a7c15));z=(z^(z>>30))*UINT64_C(0xbf58476d1ce4e5b9);z=(z^(z>>27))*UINT64_C(0x94d049bb133111eb);return z^(z>>31);}
double ag_uniform(uint64_t *rng){return (ag_random(rng)>>11)*0x1.0p-53;}
void *ag_allocate(AgMetrics *m,size_t bytes){Allocation *h=calloc(1,sizeof *h+bytes);if(!h)abort();h->bytes=bytes;m->heap_live+=bytes;if(m->heap_live>m->heap_peak)m->heap_peak=m->heap_live;return h+1;}
void ag_release(AgMetrics *m,void *p){if(p){Allocation *h=(Allocation *)p-1;m->heap_live-=h->bytes;free(h);}}
AgItem ag_item(const char *name,int type,double base){AgItem i={0};snprintf(i.name,sizeof i.name,"%s",name);i.type=type;i.base=base;i.mult=1;i.mult_type=-1;i.condition_type=-1;i.repeat=1;return i;}
static unsigned bits(uint32_t x){unsigned n=0;for(;x;x&=x-1)n++;return n;}
/* An algebraic inventory view (IDs only), never a successor observation. */
typedef struct {int ids[AG_INV],n;double cash,energy;uint32_t mask;int counts[AG_TYPES];} View;
static bool view(const AgRules *r,const AgObservation *o,AgAction a,View *v){
    if(o->plies<=0||o->turn<0||o->turn>=r->players)return false;
    const AgPlayer *p=&o->player[o->turn];memset(v,0,sizeof *v);v->n=p->count;v->cash=p->cash;v->energy=p->energy;
    memcpy(v->ids,p->inventory,(size_t)v->n*sizeof *v->ids);
    int ix=-1,iy=-1;
    for(int k=0;k<v->n;k++){if(v->ids[k]==a.x)ix=k;if(v->ids[k]==a.y)iy=k;}
    if(a.prep!=AG_NONE&&!r->preparations)return false;
    if(a.prep==AG_SELL||a.prep==AG_REPLACE){
        int sold=a.prep==AG_REPLACE?a.y:a.x,at=a.prep==AG_REPLACE?iy:ix;
        if(at<0||(r->item[sold].flags&AG_ETERNAL)||r->item[sold].sale<=0)return false;
        v->cash+=r->item[sold].sale;
        for(int k=at;k+1<v->n;k++)v->ids[k]=v->ids[k+1];
        v->n--;
    }
    if(a.prep==AG_BUY||a.prep==AG_REPLACE){
        if(a.x<0||a.x>=r->item_count||!(o->market&(1u<<a.x))||ix>=0||v->n>=r->capacity)return false;
        if(v->cash<r->item[a.x].price)return false;
        v->cash-=r->item[a.x].price;v->ids[v->n++]=a.x;
    }else if(a.prep==AG_SWAP){
        if(ix<0||iy<0||ix==iy)return false;
        int tmp=v->ids[ix];v->ids[ix]=v->ids[iy];v->ids[iy]=tmp;
    }else if(a.prep!=AG_NONE&&a.prep!=AG_SELL)return false;
    for(int k=0;k<v->n;k++)v->mask|=1u<<v->ids[k];
    if((a.use&~v->mask)||bits(a.use)>(unsigned)r->max_use)return false;
    double energy_cost=0,cash_cost=0;
    for(int k=0;k<v->n;k++)if(a.use&(1u<<v->ids[k])){
        const AgItem *it=&r->item[v->ids[k]];
        if(it->flags&AG_PASSIVE)return false;
        if((it->requires&v->mask)!=it->requires)return false;
        if((it->flags&AG_NEEDS_SALE)&&a.prep!=AG_SELL&&a.prep!=AG_REPLACE)return false;
        energy_cost+=it->energy_cost;cash_cost+=it->cash_cost;v->counts[it->type]++;
    }
    if(energy_cost>v->energy||cash_cost>v->cash)return false;
    v->energy-=energy_cost;v->cash-=cash_cost;return true;
}
bool ag_legal(const AgRules *r,const AgObservation *o,AgAction a){View v;return view(r,o,a,&v);}
bool ag_equivalent_swap(const AgRules *r,const AgObservation *o,AgAction a){
    if(a.prep!=AG_SWAP)return false;
    const AgPlayer *p=&o->player[o->turn];int lo=-1,hi=-1;
    for(int k=0;k<p->count;k++){if(p->inventory[k]==a.x)lo=k;if(p->inventory[k]==a.y)hi=k;}
    if(lo<0||hi<0)return false;
    if(lo>hi){int t=lo;lo=hi;hi=t;}
    for(int k=lo;k<=hi;k++)if((r->item[p->inventory[k]].mult!=1||r->item[p->inventory[k]].cash_step>0))return false;
    return true;
}
static bool subsets(const AgRules *r,const AgObservation *o,AgAction a,AgVisitor visit,void *ctx){
    /* Construct only subsets of present IDs after the one explicit preparation. */
    const AgPlayer *p=&o->player[o->turn];int ids[AG_INV+1],n=0;
    for(int k=0;k<p->count;k++){
        int id=p->inventory[k];if((a.prep==AG_SELL&&id==a.x)||(a.prep==AG_REPLACE&&id==a.y))continue;
        if(!(r->item[id].flags&AG_PASSIVE))ids[n++]=id;
    }
    if((a.prep==AG_BUY||a.prep==AG_REPLACE)&&!(r->item[a.x].flags&AG_PASSIVE))ids[n++]=a.x;
    for(uint32_t mask=0;mask<(1u<<n);mask++){
        if(bits(mask)>(unsigned)r->max_use)continue;
        a.use=0;
        for(int k=0;k<n;k++)if(mask&(1u<<k))a.use|=1u<<ids[k];
        if(!visit(a,ctx))return false;
    }
    return true;
}
void ag_generate(const AgRules *r,const AgObservation *o,AgVisitor visit,void *ctx){
    AgAction a={0,AG_NONE,-1,-1};if(!subsets(r,o,a,visit,ctx)||!r->preparations)return;
    const AgPlayer *p=&o->player[o->turn];
    for(int k=0;k<p->count;k++){a=(AgAction){0,AG_SELL,p->inventory[k],-1};if(!subsets(r,o,a,visit,ctx))return;}
    for(int id=0;id<r->item_count;id++)if(o->market&(1u<<id)){
        a=(AgAction){0,AG_BUY,id,-1};if(!subsets(r,o,a,visit,ctx))return;
        for(int k=0;k<p->count;k++){a=(AgAction){0,AG_REPLACE,id,p->inventory[k]};if(!subsets(r,o,a,visit,ctx))return;}
    }
    for(int i=0;i<p->count;i++)for(int j=i+1;j<p->count;j++){
        a=(AgAction){0,AG_SWAP,p->inventory[i],p->inventory[j]};if(!subsets(r,o,a,visit,ctx))return;
    }
}
static int outcome_compare(const void *a,const void *b){double x=((const AgOutcome *)a)->value,y=((const AgOutcome *)b)->value;return (x>y)-(x<y);}
bool ag_evaluate(const AgRules *r,const AgObservation *o,AgAction a,int mode,AgEvaluation *e,AgMetrics *m){
    memset(e,0,sizeof *e);View v;if(!view(r,o,a,&v))return false;
    e->valid=true;e->exact_model=true;double base[AG_TYPES]={0};int type[AG_CHANCE]={0};
    if(a.use)base[0]=o->player[o->turn].growth;
    if(mode!=AG_NO_DEPENDENCIES&&mode!=AG_SIMPLIFIED)for(int t=0;t<AG_TYPES;t++)if(v.counts[t]>=2)base[t]+=r->pair_bonus*(v.counts[t]-1);
    for(int k=0;k<v.n;k++){
        int id=v.ids[k];const AgItem *it=&r->item[id];bool selected=(a.use&(1u<<id))!=0;
        bool active=(selected&&!(it->flags&AG_HELD))||((it->flags&AG_PASSIVE)&&a.use)||((it->flags&AG_HELD)&&!selected&&a.use);
        if(!active||(it->requires&v.mask)!=it->requires)continue;
        if(it->condition_type>=0&&v.counts[it->condition_type]<it->condition_count)continue;
        bool interaction=it->requires||it->condition_type>=0||it->mult_type>=0||it->cash_step>0;
        if(mode==AG_NO_DEPENDENCIES&&interaction)continue;
        int t=it->type;double gain=it->growth;
        e->growth+=gain;v.cash+=it->cash_gain;v.energy+=it->energy_gain;
        base[t]+=it->base+(mode==AG_SIMPLIFIED?0:it->add+gain);
        if(it->cash_step>0&&mode!=AG_SIMPLIFIED)base[t]+=floor(v.cash/it->cash_step)*it->cash_factor;
        if(it->bonus!=0&&mode!=AG_SIMPLIFIED)for(int rep=0;rep<it->repeat;rep++){
            if(e->chance_count>=AG_CHANCE){e->valid=false;return false;}
            int j=e->chance_count++;e->coeff[j]=it->bonus;e->probability[j]=(it->prob_low+it->prob_high)/2;
            e->source[j]=id;e->repeat_index[j]=rep;type[j]=t;if(it->prob_low!=it->prob_high)e->exact_model=false;
        }
        if(mode!=AG_SIMPLIFIED){
            for(int t2=0;t2<AG_TYPES;t2++)if(it->mult_type<0||it->mult_type==t2)base[t2]*=it->mult;
            for(int j=0;j<e->chance_count;j++)if(it->mult_type<0||it->mult_type==type[j])e->coeff[j]*=it->mult;
        }
    }
    for(int t=0;t<AG_TYPES;t++)e->constant+=base[t];
    e->cash_delta=v.cash-o->player[o->turn].cash;e->energy_delta=v.energy-o->player[o->turn].energy;
    e->outcomes=1;e->outcome[0]=(AgOutcome){e->constant,1};
    if(mode==AG_NO_AGGREGATION){
        e->outcomes=1u<<e->chance_count;
        for(unsigned mask=0;mask<e->outcomes;mask++){
            double value=e->constant,p=1;
            for(int j=0;j<e->chance_count;j++)if(mask&(1u<<j)){value+=e->coeff[j];p*=e->probability[j];}else p*=1-e->probability[j];
            e->outcome[mask]=(AgOutcome){value,p};if(m){m->outcomes_enumerated++;m->probability_terms+=(uint64_t)e->chance_count;}
        }
    }else{
        AgOutcome tmp[AG_OUTCOMES];
        for(int j=0;j<e->chance_count;j++){
            unsigned z=e->outcomes;
            for(unsigned k=0;k<z;k++){tmp[2*k]=(AgOutcome){e->outcome[k].value,e->outcome[k].probability*(1-e->probability[j])};tmp[2*k+1]=(AgOutcome){e->outcome[k].value+e->coeff[j],e->outcome[k].probability*e->probability[j]};}
            qsort(tmp,2*z,sizeof *tmp,outcome_compare);e->outcomes=0;
            for(unsigned k=0;k<2*z;k++){
                if(tmp[k].probability==0)continue;
                if(e->outcomes&&e->outcome[e->outcomes-1].value==tmp[k].value)e->outcome[e->outcomes-1].probability+=tmp[k].probability;
                else e->outcome[e->outcomes++]=tmp[k];
            }
            if(m)m->probability_terms+=2*z;
        }
    }
    for(unsigned k=0;k<e->outcomes;k++){double prob=e->outcome[k].probability;e->expected_score+=prob*e->outcome[k].value;if(e->outcome[k].value>=r->target)e->success+=prob;}
    bool all_win=true;
    for(unsigned k=0;k<e->outcomes;k++)if(e->outcome[k].probability>0&&e->outcome[k].value<r->target)all_win=false;
    if(all_win||e->success>1)e->success=1;
    e->expected_reward=e->expected_score+r->goal_reward*e->success;return true;
}
void ag_apply(const AgRules *r,AgObservation *o,AgAction a,const AgEvaluation *e,double score,AgMetrics *m,unsigned depth){
    View v;if(!e->valid||!view(r,o,a,&v))abort();AgPlayer *p=&o->player[o->turn];p->count=0;
    for(int k=0;k<v.n;k++){int id=v.ids[k];if((a.use&(1u<<id))&&(r->item[id].flags&AG_CONSUME))continue;p->inventory[p->count++]=id;}
    if(a.prep==AG_BUY||a.prep==AG_REPLACE)o->market&=~(1u<<a.x);
    p->cash+=e->cash_delta+r->cash_refresh;p->energy+=e->energy_delta+r->energy_refresh;p->growth+=e->growth;
    p->reward+=score+(score>=r->target?r->goal_reward:0);p->goals+=score>=r->target;
    o->plies--;o->turn=(o->turn+1)%r->players;
    if(m){m->future_states++;if(depth>m->temporal_depth)m->temporal_depth=depth;}
}
double ag_utility(const AgRules *r,const AgObservation *o,int root){
    double value=o->player[root].reward+0.1*(o->player[root].cash+o->player[root].energy);
    if(r->players==2){const AgPlayer *other=&o->player[1-root];value-=other->reward+0.1*(other->cash+other->energy);}
    return value;
}
double ag_sample_model(const AgEvaluation *e,uint64_t *rng){double score=e->constant;for(int j=0;j<e->chance_count;j++)if(ag_uniform(rng)<e->probability[j])score+=e->coeff[j];return score;}
double ag_sample_environment(const AgEvaluation *e,const double p[AG_ITEMS],uint64_t seed,int ply){
    double score=e->constant;
    for(int j=0;j<e->chance_count;j++){
        uint64_t key=seed^((uint64_t)(ply+1)*UINT64_C(0xa0761d6478bd642f))^((uint64_t)(e->source[j]+1)*UINT64_C(0xe7037ed1a0b428db))^(uint64_t)e->repeat_index[j];
        if(ag_uniform(&key)<p[e->source[j]])score+=e->coeff[j];
    }
    return score;
}
bool ag_known_deterministic(const AgRules *r){for(int i=0;i<r->item_count;i++)if(r->item[i].bonus&& (r->item[i].prob_low!=r->item[i].prob_high||(r->item[i].prob_low!=0&&r->item[i].prob_low!=1)))return false;return true;}
void ag_print_action(FILE *out,AgAction a){fprintf(out,"{\"prep\":%d,\"x\":%d,\"y\":%d,\"use\":%u}",a.prep,a.x,a.y,a.use);}
