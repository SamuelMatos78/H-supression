#include "agent.h"
#include <math.h>
#include <string.h>
static void own(AgObservation *o,int id){for(int p=0;p<2;p++)o->player[p].inventory[o->player[p].count++]=id;}
bool ag_case(const char *name,unsigned variant,AgRules *r,AgObservation *o){
    memset(r,0,sizeof *r);memset(o,0,sizeof *o);snprintf(r->name,sizeof r->name,"%s",name);
    r->capacity=6;r->max_use=3;r->rounds=3;r->players=2;r->target=30;r->goal_reward=20;r->pair_bonus=8;r->energy_refresh=2;r->preparations=true;
    for(int p=0;p<2;p++){o->player[p].cash=5;o->player[p].energy=6;}
    if(!strcmp(name,"cards")){
        r->item_count=8;
        r->item[0]=ag_item("red_token",0,12);r->item[0].bonus=20;r->item[0].prob_low=r->item[0].prob_high=0.2;r->item[0].repeat=2;r->item[0].flags=AG_CONSUME;r->item[0].sale=3;
        r->item[1]=ag_item("paired_token",0,12);r->item[1].flags=AG_CONSUME;r->item[1].sale=3;
        r->item[2]=ag_item("pair_growth",1,5);r->item[2].condition_type=0;r->item[2].condition_count=2;r->item[2].growth=4;r->item[2].sale=2;
        r->item[3]=ag_item("ordered_amplifier",2,0);r->item[3].flags=AG_PASSIVE;r->item[3].mult=1.5;r->item[3].sale=4;
        r->item[4]=ag_item("sale_battery",3,0);r->item[4].flags=AG_CONSUME|AG_NEEDS_SALE;r->item[4].energy_gain=8;r->item[4].sale=1;
        r->item[5]=ag_item("market_token",0,10);r->item[5].price=4;r->item[5].sale=2;r->item[5].flags=AG_CONSUME;
        r->item[6]=ag_item("ordered_addition",2,0);r->item[6].flags=AG_PASSIVE;r->item[6].add=14;r->item[6].price=5;r->item[6].sale=2;
        r->item[7]=ag_item("held_resource",1,0);r->item[7].flags=AG_HELD;r->item[7].add=6;r->item[7].price=3;r->item[7].sale=1;
        for(int i=0;i<5;i++){own(o,i);r->item[i].energy_cost=(i<3);}
        o->market=(1u<<5)|(1u<<6)|(1u<<7);
    }else if(!strcmp(name,"economy")){
        r->item_count=7;r->max_use=2;r->target=35;r->energy_refresh=1;
        r->item[0]=ag_item("component_A",0,12);r->item[0].energy_cost=1;r->item[0].sale=5;
        r->item[1]=ag_item("sale_scrap",1,1);r->item[1].sale=8;
        r->item[2]=ag_item("cash_engine",2,0);r->item[2].flags=AG_PASSIVE;r->item[2].cash_step=5;r->item[2].cash_factor=15;r->item[2].sale=2;
        r->item[3]=ag_item("conditional_energy",3,0);r->item[3].flags=AG_CONSUME|AG_NEEDS_SALE;r->item[3].energy_gain=20;r->item[3].sale=1;
        r->item[4]=ag_item("type_A_tripler",2,0);r->item[4].flags=AG_PASSIVE;r->item[4].mult=3;r->item[4].mult_type=0;r->item[4].price=8;r->item[4].sale=4;
        r->item[5]=ag_item("dependent_reactor",0,40);r->item[5].requires=1u;r->item[5].energy_cost=3;r->item[5].price=6;r->item[5].sale=3;
        r->item[6]=ag_item("persistent_training",3,1);r->item[6].growth=5;r->item[6].cash_cost=1;r->item[6].price=2;r->item[6].sale=1;
        for(int i=0;i<4;i++)own(o,i);
        o->market=(1u<<4)|(1u<<5)|(1u<<6);
        for(int p=0;p<2;p++){o->player[p].cash=0;o->player[p].energy=2;}
    }else if(!strcmp(name,"stochastic")||!strcmp(name,"unknown")){
        r->item_count=6;r->capacity=5;r->max_use=2;r->target=35;r->energy_refresh=1;
        r->item[0]=ag_item("safe_generator",0,14);r->item[0].sale=3;r->item[0].energy_cost=1;
        r->item[1]=ag_item("double_trial",0,0);r->item[1].bonus=30;r->item[1].prob_low=r->item[1].prob_high=0.25;r->item[1].repeat=2;r->item[1].sale=2;r->item[1].energy_cost=2;
        r->item[2]=ag_item("slow_growth",1,4);r->item[2].growth=3;r->item[2].sale=2;
        r->item[3]=ag_item("dependent_boost",2,0);r->item[3].flags=AG_PASSIVE;r->item[3].mult=2;r->item[3].mult_type=0;r->item[3].requires=1u;r->item[3].sale=4;
        r->item[4]=ag_item("market_trial",1,5);r->item[4].bonus=20;r->item[4].prob_low=r->item[4].prob_high=0.5;r->item[4].price=4;r->item[4].sale=2;
        r->item[5]=ag_item("one_use_reserve",3,45);r->item[5].flags=AG_CONSUME;r->item[5].price=5;r->item[5].sale=1;r->item[5].energy_cost=3;
        for(int i=0;i<4;i++)own(o,i);
        o->market=(1u<<4)|(1u<<5);
        if(!strcmp(name,"unknown"))for(int i=0;i<6;i++)if(r->item[i].bonus){r->item[i].prob_low=0.1;r->item[i].prob_high=0.9;}
    }else if(!strcmp(name,"trap")||!strcmp(name,"tiny")){
        r->item_count=3;r->capacity=3;r->max_use=1;r->target=10;r->goal_reward=5;r->pair_bonus=0;r->energy_refresh=0;
        r->item[0]=ag_item("immediate_temptation",0,18);r->item[0].flags=AG_CONSUME;r->item[0].energy_cost=2;
        r->item[1]=ag_item("investment",1,0);r->item[1].flags=AG_CONSUME;r->item[1].energy_cost=2;r->item[1].cash_gain=6;r->item[1].energy_gain=4;
        r->item[2]=ag_item("shared_reactor",0,60);r->item[2].price=6;r->item[2].energy_cost=1;
        own(o,0);own(o,1);o->market=1u<<2;
        for(int p=0;p<2;p++){o->player[p].cash=0;o->player[p].energy=2;}
        if(!strcmp(name,"tiny"))r->players=1;
    }else if(!strcmp(name,"tiny_chance")){
        r->item_count=2;r->capacity=2;r->max_use=1;r->rounds=2;r->players=1;r->target=6;r->goal_reward=3;r->pair_bonus=0;r->preparations=false;
        r->item[0]=ag_item("safe",0,5);r->item[1]=ag_item("uncertain",1,0);r->item[1].bonus=12;r->item[1].prob_low=r->item[1].prob_high=0.5;
        own(o,0);own(o,1);
    }else if(!strcmp(name,"future_renewal")||!strcmp(name,"future_deterministic")||!strcmp(name,"future_shared")){
        int n=variant>=2&&variant<=8?(int)variant:8;
        bool shared=!strcmp(name,"future_shared"),deterministic=!strcmp(name,"future_deterministic");
        r->item_count=n+(shared?2:0);r->capacity=n+(shared?2:0);r->max_use=n;
        r->rounds=10;r->target=3*n+0.5*((1u<<n)-1);r->pair_bonus=0;
        r->energy_refresh=shared?4:0;r->preparations=shared;
        for(int p=0;p<2;p++){o->player[p].cash=shared?12:0;o->player[p].energy=shared?8:0;}
        for(int i=0;i<n;i++){
            char label[32];snprintf(label,sizeof label,"source_%d",i);
            r->item[i]=ag_item(label,i%4,3);r->item[i].energy_cost=shared?1:0;
            if(deterministic)r->item[i].base+=0.5*(1u<<i);
            else {r->item[i].bonus=(1u<<i);r->item[i].prob_low=r->item[i].prob_high=0.5;}
            own(o,i);
        }
        if(shared){
            r->item[n]=ag_item("shared_amplifier",2,0);r->item[n].flags=AG_PASSIVE;r->item[n].mult=1.5;r->item[n].price=10;
            r->item[n+1]=ag_item("shared_training",3,6);r->item[n+1].flags=AG_PASSIVE;r->item[n+1].growth=2;r->item[n+1].price=5;
            o->market=(1u<<n)|(1u<<(n+1));
        }
    }else if(!strcmp(name,"branching")){
        int n=variant>=4&&variant<=12?(int)variant:8;r->item_count=n;r->capacity=12;r->max_use=4;r->rounds=1;r->players=1;r->target=70;r->energy_refresh=0;
        for(int i=0;i<n;i++){char label[32];snprintf(label,sizeof label,"resource_%d",i);r->item[i]=ag_item(label,i%4,3+i);r->item[i].sale=1;if(i==n-1)r->item[i].mult=2;own(o,i);}
    }else return false;
    if(strncmp(name,"future_",7)&&strcmp(name,"branching")&&strcmp(name,"tiny")&&strcmp(name,"tiny_chance")){
        /* Public deterministic variants, identical for both seats. No outcome RNG. */
        r->target+=(int)(variant%3)-1;
        for(int i=0;i<r->item_count;i++)if(r->item[i].base>2)r->item[i].base+=(int)((variant*(unsigned)(i+7)+(unsigned)i*11)%5)-2;
        if(variant&1u)for(int p=0;p<2;p++)if(o->player[p].count>2){int tmp=o->player[p].inventory[0];o->player[p].inventory[0]=o->player[p].inventory[2];o->player[p].inventory[2]=tmp;}
    }
    if(r->players==1)memset(&o->player[1],0,sizeof o->player[1]);
    o->plies=r->rounds*r->players;return true;
}
void ag_save(FILE *f,const AgRules *r,const AgObservation *o){
    fprintf(f,"AG1 %s\n%d %d %d %d %d %.17g %.17g %.17g %.17g %.17g %d\n",r->name,r->item_count,r->capacity,r->max_use,r->rounds,r->players,r->target,r->goal_reward,r->pair_bonus,r->energy_refresh,r->cash_refresh,r->preparations);
    for(int k=0;k<r->item_count;k++){const AgItem *i=&r->item[k];fprintf(f,"%s %d %d %d %d %d %d %.17g %.17g %.17g %.17g %.17g %.17g %d %d %d %d %d %.17g %.17g %.17g %.17g %u\n",i->name,i->type,i->flags,i->price,i->sale,i->energy_cost,i->cash_cost,i->base,i->add,i->mult,i->bonus,i->prob_low,i->prob_high,i->repeat,i->mult_type,i->condition_type,i->condition_count,i->cash_step,i->cash_factor,i->energy_gain,i->cash_gain,i->growth,i->requires);}
    fprintf(f,"%u %d %d\n",o->market,o->turn,o->plies);
    for(int k=0;k<2;k++){const AgPlayer *p=&o->player[k];fprintf(f,"%d %.17g %.17g %.17g %.17g %d",p->count,p->cash,p->energy,p->growth,p->reward,p->goals);for(int j=0;j<p->count;j++)fprintf(f," %d",p->inventory[j]);fputc('\n',f);}
}
bool ag_load(FILE *f,AgRules *r,AgObservation *o){
    memset(r,0,sizeof *r);memset(o,0,sizeof *o);char magic[8];int prep;
    if(fscanf(f,"%7s %31s",magic,r->name)!=2||strcmp(magic,"AG1"))return false;
    if(fscanf(f,"%d %d %d %d %d %lf %lf %lf %lf %lf %d",&r->item_count,&r->capacity,&r->max_use,&r->rounds,&r->players,&r->target,&r->goal_reward,&r->pair_bonus,&r->energy_refresh,&r->cash_refresh,&prep)!=11)return false;
    if(r->item_count<1||r->item_count>AG_ITEMS||r->capacity<1||r->capacity>AG_INV||r->max_use<1||r->max_use>r->capacity||r->rounds<1||r->rounds>10||r->players<1||r->players>2)return false;
    r->preparations=prep!=0;
    if(!isfinite(r->target)||r->target<=0||!isfinite(r->goal_reward)||r->goal_reward<0||!isfinite(r->pair_bonus)||r->pair_bonus<0||!isfinite(r->energy_refresh)||r->energy_refresh<0||!isfinite(r->cash_refresh)||r->cash_refresh<0)return false;
    for(int k=0;k<r->item_count;k++){AgItem *i=&r->item[k];
        if(fscanf(f,"%31s %d %d %d %d %d %d %lf %lf %lf %lf %lf %lf %d %d %d %d %d %lf %lf %lf %lf %u",i->name,&i->type,&i->flags,&i->price,&i->sale,&i->energy_cost,&i->cash_cost,&i->base,&i->add,&i->mult,&i->bonus,&i->prob_low,&i->prob_high,&i->repeat,&i->mult_type,&i->condition_type,&i->condition_count,&i->cash_step,&i->cash_factor,&i->energy_gain,&i->cash_gain,&i->growth,&i->requires)!=23)return false;
        if(i->type<0||i->type>=AG_TYPES||i->price<0||i->sale<0||i->energy_cost<0||i->cash_cost<0||!isfinite(i->base)||!isfinite(i->add)||!isfinite(i->mult)||i->mult<=0||i->mult>10||!isfinite(i->bonus)||i->bonus<0||!isfinite(i->prob_low)||!isfinite(i->prob_high)||i->prob_low<0||i->prob_high>1||i->prob_low>i->prob_high||i->repeat<1||i->repeat>2||i->mult_type<-1||i->mult_type>=AG_TYPES||i->condition_type<-1||i->condition_type>=AG_TYPES)return false;
    }
    for(int k=0;k<r->item_count;k++){const AgItem *i=&r->item[k];if(i->flags<0||i->flags>31||i->condition_count<0||i->cash_step<0||!isfinite(i->cash_factor)||!isfinite(i->energy_gain)||!isfinite(i->cash_gain)||!isfinite(i->growth)||i->energy_gain<0||i->cash_gain<0||i->growth<0||(i->requires>>r->item_count))return false;}
    /* Bounded experimental model: every configuration must fit the exact
       outcome workspace, even if all probabilistic effects activate together. */
    int chance_terms=0;
    for(int k=0;k<r->item_count;k++)if(r->item[k].bonus)chance_terms+=r->item[k].repeat;
    if(chance_terms>AG_CHANCE)return false;
    if(fscanf(f,"%u %d %d",&o->market,&o->turn,&o->plies)!=3||o->turn<0||o->turn>=r->players||o->plies<1||o->plies>AG_PLIES)return false;
    for(int k=0;k<2;k++){AgPlayer *p=&o->player[k];if(fscanf(f,"%d %lf %lf %lf %lf %d",&p->count,&p->cash,&p->energy,&p->growth,&p->reward,&p->goals)!=6||p->count<0||p->count>r->capacity||!isfinite(p->cash)||!isfinite(p->energy)||!isfinite(p->growth)||!isfinite(p->reward)||p->cash<0||p->energy<0)return false;
        uint32_t mask=0;for(int j=0;j<p->count;j++){if(fscanf(f,"%d",&p->inventory[j])!=1||p->inventory[j]<0||p->inventory[j]>=r->item_count||(mask&(1u<<p->inventory[j])))return false;mask|=1u<<p->inventory[j];}
    }
    if(o->market>>r->item_count)return false;
    return true;
}
