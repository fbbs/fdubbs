/* ----------------------------------- */
/* pip.c  养小鸡程式                   */
/* 原作者: dsyan   改写者: fennet      */
/* 图图 by tiball.bbs@bbs.nhctc.edu.tw */
/* ----------------------------------- */

//#define getdata(a, b, c , d, e, f, g) getdata(a,b,c,d,e,f,NULL,g)
//#define pressanykey(a) prints(a);pressanykey();

#include "bbs.h"
#include <time.h>
#include "pip.h"
struct chicken d;
time_t start_time;
time_t lasttime;

#ifndef MAPLE
extern char BoardName[];
#endif  // END MAPLE
void temppress(char *s)
{
	move(23, 0); clrtoeol();
	prints(s);
	egetch();
}

/*游戏主程式*/
//int p_pipple()
int mod_default()
{
 FILE *fs;
 time_t now;
 long smoney;
 int pipkey;
 int ok;
 char genbuf[200];

#ifdef MAPLE
 setutmpmode(CHICKEN);
 more("src/maple/pipgame/pip.welcome",YEA);
#else
 modify_user_mode( CHICKEN );
 refresh();
 move(1,0);
 clrtobot();
 //rawmore("game/pipgame/pip.welcome",YEA,0,0,MM_FILE); 
//ansimore("game/pipgame/pip.welcome", NA);
//egetch();
#endif  // END MAPLE
 showtitle("电子养小鸡", BoardName);
 srandom(time(0));
#ifdef MAPLE
 sprintf(genbuf,"home/%s/new_chicken",cuser.userid);
#else
 sprintf(genbuf,"home/%c/%s/new_chicken",toupper(cuser.userid[0]),cuser.userid);
#endif  // END MAPLE
 
 pip_read_file();
 if((fs=fopen(genbuf, "r")) == NULL)
 {
//   show_system_pic(11); /* 暂时用进游戏的画面来代替 */
 //  move(b_lines,0);
ansimore("game/pipgame/pip.welcome", NA);
   pipkey=egetch();   
   if(pipkey=='Q' || pipkey=='q')
     return 0;
   if(d.death!=0 || !d.name[0])
   {
       if(!pip_new_game()) return 0;
   }      
 }
 else
 {
//   show_system_pic(12);
 //  move(b_lines,0);
ansimore("game/pipgame/pip.welcome", NA);
   pipkey=egetch();   
   if(pipkey=='R' || pipkey=='r')
     pip_read_backup();
   else if(pipkey=='Q' || pipkey=='q')
     return 0;
   if(d.death!=0 || !d.name[0])
     {
       if(!pip_new_game()) return 0;
     }
 }
 
 lasttime=time(0);
 start_time=time(0);
 /*pip_do_menu(0,0,pipmainlist);*/
 pip_main_menu();
 d.bbtime+=time(0)-start_time;
 pip_write_file();
 return 0;
}

/*时间表示法*/
char*
dsyan_time(const time_t *t)
{
  struct tm *tp;
  static char ans[9];

  tp = localtime(t);
  sprintf(ans, "%02d/%02d/%02d", tp->tm_year%100, tp->tm_mon + 1,tp->tm_mday);
  return ans;
}

/*新游戏的设定*/
int
pip_new_game()
{
  char buf[256];
  time_t now;
  char *pipsex[3]={"？","♂","♀"};
  struct tm *ptime;
  ptime = localtime(&now);
  
  if(d.death==1 && !(!d.name[0]))
  {
     clear();
     showtitle("外星战斗鸡", BoardName); 
     move(4,6);
     prints("欢迎来到 [1;5;33m星空生物科技研究院[0m");
     move(6,6);
     prints("经我们调查显示  先前你有养过小鸡喔  可是被你养死了...");
     move(8,6);
     if(d.liveagain<4)
     {
       prints("我们可以帮你帮小鸡复活  但是需要付出一点代价");
#ifdef MAPLE
       getdata(10, 6, "你要我们让他重生吗? [y/N]:", buf, 2, 1, 0);
#else
       getdata(10, 6, "你要我们让他重生吗? [y/N]:", buf, 2, DOECHO, YEA);
#endif  // END MAPLE
       if(buf[0]=='y' || buf[0]=='Y')
       {
         pip_live_again();
       }
     }
     else if(d.liveagain>=4)
     {
       prints("可是你复活手术太多次了  小鸡身上都是开刀痕迹");     
       move(10,6);
       prints("我们找不到可以手术的地方了  所以....");
       pressanykey("重新再来吧....唉....");    
     }
  }
  if(d.death!=0 || !d.name[0])
  {
    clear();
    showtitle("外星战斗鸡", BoardName);   
    /*小鸡命名*/
    strcpy(buf,"贝贝");
#ifdef MAPLE
    getdata(2, 3, "帮小鸡取个好听的名字吧(请不要有空格):", buf, 11, 1, 0);
#else
    getdata(2, 3, "帮小鸡取个好听的名字吧(请不要有空格):", buf, 11, DOECHO, NA);
#endif  // END MAPLE
    if(!buf[0]) return 0;
    strcpy(d.name,buf);
    /*1:公 2:母 */
#ifdef MAPLE
    getdata(4, 3, "[Boy]小公鸡♂ or [Girl]小母鸡♀ [b/G]", buf, 2, 1, 0);
#else
    getdata(4, 3, "[Boy]小公鸡♂ or [Girl]小母鸡♀ [b/G]", buf, 2, DOECHO, YEA);
#endif  // END MAPLE
    if(buf[0]=='b' || buf[0]=='B')
    {
      d.sex=1;
    }  
    else
    {
      d.sex=2; 
    }        
    move(6,3);
    prints("星空战斗鸡的游戏现今分成两种玩法");
    move(7,3);
    prints("选有结局会在小鸡20岁时结束游戏，并告知小鸡後续的发展");
    move(8,3);
    prints("选没有结局则一直养到小鸡死亡才结束游戏....");
    /*1:不要且未婚 4:要且未婚 */
#ifdef MAPLE
    getdata(9, 3, "你希望小鸡游戏是否要有20岁结局? [Y/n]", buf, 2, 1, 0);
#else
    getdata(9, 3, "你希望小鸡游戏是否要有20岁结局? [Y/n]", buf, 2, DOECHO, YEA);
#endif  // END MAPLE
    if(buf[0]=='n' || buf[0]=='N')
    {
      d.wantend=1;
    }  
    else
    {
      d.wantend=4; 
    }        
    /*开头画面*/
    show_basic_pic(0);
    pressanykey("小鸡终於诞生了，请好好爱他....");

    /*开头设定*/
    now=time(0);
    strcpy(d.birth,dsyan_time(&now));
    d.bbtime=0;

    /*基本资料*/
    d.year=ptime->tm_year%100;
    d.month=ptime->tm_mon + 1;
    d.day=ptime->tm_mday;
    d.death=d.nodone=d.relation=0;
    d.liveagain=d.dataB=d.dataC=d.dataD=d.dataE=0;
          
    /*身体参数*/
    d.hp=rand()%15+20;
    d.maxhp=rand()%20+20;
    if(d.hp>d.maxhp) d.hp=d.maxhp;
    d.weight=rand()%10+50;
    d.tired=d.sick=d.shit=d.wrist=0;
    d.bodyA=d.bodyB=d.bodyC=d.bodyD=d.bodyE=0;
  
    /*评价参数*/
    d.social=d.family=d.hexp=d.mexp=0;
    d.tmpA=d.tmpB=d.tmpC=d.tmpD=d.tmpE=0;
         
    /*战斗参数*/
    d.mp=d.maxmp=d.attack=d.resist=d.speed=d.hskill=d.mskill=d.mresist=0;
    d.magicmode=d.fightB=d.fightC=d.fightD=d.fightE=0;
  
    /*武器参数*/
    d.weaponhead=d.weaponrhand=d.weaponlhand=d.weaponbody=d.weaponfoot=0;
    d.weaponA=d.weaponB=d.weaponC=d.weaponD=d.weaponE=0;
    
    /*能力参数*/
    d.toman=d.character=d.love=d.wisdom=d.art=d.etchics=0;
    d.brave=d.homework=d.charm=d.manners=d.speech=d.cookskill=0;
    d.learnA=d.learnB=d.learnC=d.learnD=d.learnE=0;
  
    /*状态数值*/
    d.happy=rand()%10+20;
    d.satisfy=rand()%10+20;
    d.fallinlove=d.belief=d.offense=d.affect=0;
    d.stateA=d.stateB=d.stateC=d.stateD=d.stateE=0;

    /*食物参数:食物 零食 药品 大补丸*/
    d.food=10;
    d.medicine=d.cookie=d.bighp=2;
    d.ginseng=d.snowgrass=d.eatC=d.eatD=d.eatE=0;

    /*物品参数:书 玩具*/
    d.book=d.playtool=0;
    d.money=1500;
    d.thingA=d.thingB=d.thingC=d.thingD=d.thingE=0;

    /*猜拳参数:赢 负*/
    d.winn=d.losee=0;

    /*参见王臣*/
    d.royalA=d.royalB=d.royalC=d.royalD=d.royalE=0;
    d.royalF=d.royalG=d.royalH=d.royalI=d.royalJ=0;
    d.seeroyalJ=1;
    d.seeA=d.seeB=d.seeC=d.seeD=d.seeE;
    /*接受求婚爱人*/        
    d.lover=0;
    /*0:没有 1:魔王 2:龙族 3:A 4:B 5:C 6:D 7:E */
    d.classA=d.classB=d.classC=d.classD=d.classE=0;
    d.classF=d.classG=d.classH=d.classI=d.classJ=0;
    d.classK=d.classL=d.classM=d.classN=d.classO=0;

    d.workA=d.workB=d.workC=d.workD=d.workE=0;
    d.workF=d.workG=d.workH=d.workI=d.workJ=0;
    d.workK=d.workL=d.workM=d.workN=d.workO=0;
    d.workP=d.workQ=d.workR=d.workS=d.workT=0;
    d.workU=d.workV=d.workW=d.workX=d.workY=d.workZ=0;
    /*养鸡记录*/
    now=time(0);
    sprintf(buf, "[1;36m%s %-11s养了一只叫 [%s] 的 %s 小鸡 [0m\n", Cdate(&now), cuser.userid,d.name,pipsex[d.sex]);
    pip_log_record(buf);
  }  
  pip_write_file();
  return 1;
}

/*小鸡死亡函式*/
pipdie(msg,mode)
char *msg;
int mode;
{
 char buf[100];
 char genbuf[200];
 time_t now;
 clear();
 showtitle("电子养小鸡", BoardName); 
 if(mode==1)
 {
   show_die_pic(1);
   pressanykey("死神来带走小鸡了");
   clear();
   showtitle("电子养小鸡", BoardName); 
   show_die_pic(2);
   move(14,20);
   prints("可怜的小鸡[1;31m%s[m",msg);
   pressanykey("星空哀悼中....");
 }
 else if(mode==2)
 {
   show_die_pic(3);
   pressanykey("呜呜呜..我被丢弃了.....");
 } 
 else if(mode==3)
 {
   show_die_pic(0);
   pressanykey("游戏结束罗.."); 
 }
 
 now=time(0);
 sprintf(genbuf, "[1;31m%s %-11s的小鸡 [%s] %s[m\n", Cdate(&now), cuser.userid,d.name, msg);
 pip_log_record(genbuf);
 pip_write_file();
}


/*pro:机率 base:底数 mode:类型 mul:加权100=1 cal:加减*/
int
count_tired(prob,base,mode,mul,cal)
int prob,base;
char *mode;
int mul;
int cal;
{
 int tiredvary=0;
 int tm;
 /*time_t now;*/
 tm=(time(0)-start_time+d.bbtime)/60/30;
 if(mode=="Y")
 {
  if(tm>=0 && tm <=3)
  {
     if(cal==1)
        tiredvary=(rand()%prob+base)*d.maxhp/(d.hp+0.8*d.hp)*120/100;
     else if(cal==0)
        tiredvary=(rand()%prob+base)*4/3;
  }
  else if(tm >=4 && tm <=7)
  {
     if(cal==1)
        tiredvary=(rand()%prob+base)*d.maxhp/(d.hp+0.8*d.hp);
     else if(cal==0)
        tiredvary=(rand()%prob+base)*3/2;
  }
  else if(tm >=8 && tm <=10)
  {
     if(cal==1)
        tiredvary=(rand()%prob+base)*d.maxhp/(d.hp+0.8*d.hp)*110/100;
     else if(cal==0)
        tiredvary=(rand()%prob+base)*5/4;
  }
  else if(tm >=11)
  {
     if(cal==1)
        tiredvary=(rand()%prob+base)*d.maxhp/(d.hp+0.8*d.hp)*150/100;
     else if(cal==0)
        tiredvary=(rand()%prob+base)*1;
  }
 }
 else if(mode=="N")
 {
  tiredvary=rand()%prob+base;
 }

 if(cal==1)
 {
   d.tired+=(tiredvary*mul/100);
   if(d.tired>100)
     d.tired=100;
 }
 else if(cal==0)
 {
   d.tired-=(tiredvary*mul/100);
   if(d.tired<=0)
     {d.tired=0;}
 }
 tiredvary=0;
 return;
}



