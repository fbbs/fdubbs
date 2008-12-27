/*---------------------------------------------------------------------------*/
/* 系统选单:个人资料  小鸡放生  特别服务                                     */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include <time.h>
#include "bbs.h"
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;
//#define getdata(a, b, c , d, e, f, g) getdata(a,b,c,d,e,f,NULL,g)

char weaponhead[7][10]={
"没有装备",
"塑胶帽子", 
"牛皮小帽",
"  安全帽",
"钢铁头盔",
"魔法发箍",
"黄金圣盔"};


char weaponrhand[10][10]={
"没有装备",
"大木棒",  
"金属扳手",
"青铜剑",  
"晴雷剑", 
"蝉翼刀", 
"忘情剑", 
"狮头宝刀",
"屠龙刀",  
"黄金圣杖"
};  

char weaponlhand[8][10]={
"没有装备",
"大木棒", 
"金属扳手",
"木盾",
"不锈钢盾",
"白金之盾",
"魔法盾",
"黄金圣盾"
};


char weaponbody[7][10]={
"没有装备",
"塑胶胄甲",
"特级皮甲",
"钢铁盔甲",
"魔法披风",
"白金盔甲",
"黄金圣衣"};

char weaponfoot[8][12]={
"没有装备",
"塑胶拖鞋",
"东洋木屐",
"特级雨鞋",
"NIKE运动鞋",
"鳄鱼皮靴",
"飞天魔靴",
"黄金圣靴"
};

int 
pip_system_freepip()
{
      char buf[256];
      move(b_lines-1, 0);
      clrtoeol();
#ifdef MAPLE
      getdata(b_lines-1,1, "真的要放生吗？(y/N)", buf, 2, 1, 0);
#else
      getdata(b_lines-1,1, "真的要放生吗？(y/N)", buf, 2, DOECHO, YEA);
#endif  // END MAPLE
      if (buf[0]!='y'&&buf[0]!='Y') return 0;
      sprintf(buf,"%s 被狠心的 %s 丢掉了~",d.name,cuser.userid);
      pressanykey(buf);
      d.death=2;
      pipdie("[1;31m被狠心丢弃:~~[0m",2);
      return 0;
}


int
pip_system_service()
{
     int pipkey;
     int oldchoice;
     char buf[200];
     char oldname[21];
     time_t now;     

     move(b_lines, 0);
     clrtoeol();
     move(b_lines,0);
     prints("[1;44m  服务项目  [46m[1]命名大师 [2]变性手术 [3]结局设局                                [0m");
     pipkey=egetch();
     
     switch(pipkey)
     {
     case '1':
       move(b_lines-1,0);
       clrtobot();
#ifdef MAPLE
       getdata(b_lines-1, 1, "帮小鸡重新取个好名字：", buf, 11, DOECHO,NULL);
#else
       getdata(b_lines-1, 1, "帮小鸡重新取个好名字：", buf, 11, DOECHO,YEA);
#endif  // END MAPLE
       if(!buf[0])
       {
         pressanykey("等一下想好再来好了  :)");
         break;
       }
       else
       {
        strcpy(oldname,d.name);
        strcpy(d.name,buf);
        /*改名记录*/
        now=time(0);
        sprintf(buf, "[1;37m%s %-11s把小鸡 [%s] 改名成 [%s] [0m\n", Cdate(&now), cuser.userid,oldname,d.name);
        pip_log_record(buf);
        pressanykey("嗯嗯  换一个新的名字喔...");
       }
       break;
       
     case '2':  /*变性*/
       move(b_lines-1,0);
       clrtobot();
       /*1:公 2:母 */
       if(d.sex==1)
       { 
         oldchoice=2; /*公-->母*/
         move(b_lines-1, 0);
         prints("[1;33m将小鸡由[32m♂[33m变性成[35m♀[33m的吗？ [37m[y/N][0m");
       }
       else
       { 
         oldchoice=1; /*母-->公*/
         move(b_lines-1, 0); 
         prints("[1;33m将小鸡由[35m♀[33m变性成[35m♂[33m的吗？ [37m[y/N][0m");
       }
       move(b_lines,0);
       prints("[1;44m  服务项目  [46m[1]命名大师 [2]变性手术 [3]结局设局                                [0m");
       pipkey=egetch();
       if(pipkey=='Y' || pipkey=='y')
       {
         /*改名记录*/
         now=time(0);
         if(d.sex==1)
           sprintf(buf,"[1;37m%s %-11s把小鸡 [%s] 由♂变性成♀了[0m\n",Cdate(&now), cuser.userid,d.name);
         else
           sprintf(buf,"[1;37m%s %-11s把小鸡 [%s] 由♀变性成♂了[0m\n",Cdate(&now), cuser.userid,d.name);           
         pip_log_record(buf);
         pressanykey("变性手术完毕...");       
         d.sex=oldchoice;
       }  
       break;
       
     case '3':
       move(b_lines-1,0);
       clrtobot();
       /*1:不要且未婚 4:要且未婚 */
       oldchoice=d.wantend;
       if(d.wantend==1 || d.wantend==2 || d.wantend==3)
       { 
         oldchoice+=3; /*没有-->有*/
         move(b_lines-1, 0); 
         prints("[1;33m将小鸡游戏改成[32m[有20岁结局][33m? [37m[y/N][0m");
	 sprintf(buf,"小鸡游戏设定成[有20岁结局]..");         
       }
       else
       { 
         oldchoice-=3; /*有-->没有*/
         move(b_lines-1, 0); 
         prints("[1;33m将小鸡游戏改成[32m[没有20岁结局][33m? [37m[y/N][0m");
         sprintf(buf,"小鸡游戏设定成[没有20岁结局]..");
       }
       move(b_lines,0);
       prints("[1;44m  服务项目  [46m[1]命名大师 [2]变性手术 [3]结局设局                                [0m");
       pipkey=egetch();
       if(pipkey=='Y' || pipkey=='y')
       {
         d.wantend=oldchoice;
         pressanykey(buf);
       }  
       break;     
     } 
     return 0;
}

int
pip_data_list()  /*看小鸡个人详细资料*/
{
  char buf[256];
  char inbuf1[20];
  char inbuf2[20];
  int tm;
  int pipkey;
  int page=1;
  
  tm=(time(0)-start_time+d.bbtime)/60/30;

  clear();  
  move(1,0);
  prints("       [1;33m┏━━━    ━━━  ┏━━━┓  ━━━  [m\n");
  prints("       [0;37m┃      ┃┃ ━   ┃┗┓┏━┛┃ ━   ┃[m\n");
  prints("       [1;37m┃      ┃┃┏┓  ┃  ┃┃    ┃┏┓  ┃[m\n");
  prints("       [1;34m┗━━━  ┗┛┗━┛  ┗┛    ┗┛┗━┛[32m......................[m");
  do
  { clrchyiuan(5,23);
    switch(page)
    {
     case 1:
       move(5,0);
       sprintf(buf,
       "[1;31m ┌┤[41;37m 基本资料 [0;1;31m├—————————————————————————————┐[m\n");  
       prints(buf);
  
       sprintf(buf,
       "[1;31m │[33m＃姓    名 :[37m %-10s [33m＃生    日 :[37m %02d/%02d/%02d   [33m＃年    纪 :[37m %-2d         [31m│[m\n",
       d.name,d.year%100,d.month,d.day,tm);
       prints(buf);  
  
       sprintf(inbuf1,"%d/%d",d.hp,d.maxhp);  
       sprintf(inbuf2,"%d/%d",d.mp,d.maxmp);  
       sprintf(buf,
       "[1;31m │[33m＃体    重 :[37m %-5d(米克)[33m＃体    力 :[37m %-11s[33m＃法    力 :[37m %-11s[31m│[m\n",
       d.weight,inbuf1,inbuf2);
       prints(buf);  
  
       sprintf(buf,
       "[1;31m │[33m＃疲    劳 :[37m %-3d        [33m＃病    气 :[37m %-3d        [33m＃脏    脏 :[37m %-3d        [31m│[m\n",
       d.tired,d.sick,d.shit);
       prints(buf);  
   
       sprintf(buf,  
       "[1;31m │[33m＃腕    力 :[37m %-7d    [33m＃亲子关系 :[37m %-7d    [33m＃金    钱 :[37m %-11d[31m│[m\n",
       d.wrist,d.relation,d.money);
       prints(buf);  
  
       sprintf(buf,  
       "[1;31m ├┤[41;37m 能力资料 [0;1;31m├—————————————————————————————┤[m\n");
       prints(buf);  
   
       sprintf(buf,   
       "[1;31m │[33m＃气    质 :[37m %-10d [33m＃智    力 :[37m %-10d [33m＃爱    心 :[37m %-10d [31m│[m\n",
       d.character,d.wisdom,d.love);
       prints(buf);  
   
       sprintf(buf, 
       "[1;31m │[33m＃艺    术 :[37m %-10d [33m＃道    德 :[37m %-10d [33m＃家    事 :[37m %-10d [31m│[m\n",
       d.art,d.etchics,d.homework);
       prints(buf);  
 
       sprintf(buf, 
       "[1;31m │[33m＃礼    仪 :[37m %-10d [33m＃应    对 :[37m %-10d [33m＃烹    饪 :[37m %-10d [31m│[m\n",
       d.manners,d.speech,d.cookskill);
       prints(buf);    
 
       sprintf(buf,  
       "[1;31m ├┤[41;37m 状态资料 [0;1;31m├—————————————————————————————┤[m\n");
       prints(buf);  
 
       sprintf(buf, 
       "[1;31m │[33m＃快    乐 :[37m %-10d [33m＃满    意 :[37m %-10d [33m＃人    际 :[37m %-10d [31m│[m\n",
       d.happy,d.satisfy,d.toman);
       prints(buf);
  
       sprintf(buf, 
       "[1;31m │[33m＃魅    力 :[37m %-10d [33m＃勇    敢 :[37m %-10d [33m＃信    仰 :[37m %-10d [31m│[m\n",
       d.charm,d.brave,d.belief);
       prints(buf);  

       sprintf(buf, 
       "[1;31m │[33m＃罪    孽 :[37m %-10d [33m＃感    受 :[37m %-10d [33m            [37m            [31m│[m\n",
       d.offense,d.affect);
       prints(buf);  

       sprintf(buf, 
       "[1;31m ├┤[41;37m 评价资料 [0;1;31m├—————————————————————————————┤[m\n");
       prints(buf);  

       sprintf(buf, 
       "[1;31m │[33m＃社交评价 :[37m %-10d [33m＃战斗评价 :[37m %-10d [33m＃魔法评价 :[37m %-10d [31m│[m\n",
       d.social,d.hexp,d.mexp);
       prints(buf);  

       sprintf(buf, 
       "[1;31m │[33m＃家事评价 :[37m %-10d [33m            [37m            [33m            [37m            [31m│[m\n",
       d.family);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m └————————————————————————————————————┘[m\n");
       prints(buf);  
       
       move(b_lines-1,0);       
       sprintf(buf, 
       "                                                              [1;36m第一页[37m/[36m共二页[m\n");
       prints(buf);  
       break;

     case 2:
       move(5,0);
       sprintf(buf, 
       "[1;31m ┌┤[41;37m 物品资料 [0;1;31m├—————————————————————————————┐[m\n");
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m │[33m＃食    物 :[37m %-10d [33m＃零    食 :[37m %-10d [33m＃大 补 丸 :[37m %-10d [31m│[m\n",
       d.food,d.cookie,d.bighp);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m │[33m＃药    草 :[37m %-10d [33m＃书    本 :[37m %-10d [33m＃玩    具 :[37m %-10d [31m│[m\n",
       d.medicine,d.book,d.playtool);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ├┤[41;37m 游戏资料 [0;1;31m├—————————————————————————————┤[m\n");
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m │[33m＃猜 拳 赢 :[37m %-10d [33m＃猜 拳 输 :[37m %-10d                         [31m│[m\n",
       d.winn,d.losee);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m ├┤[41;37m 武力资料 [0;1;31m├—————————————————————————————┤[m\n");
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m │[33m＃攻 击 力 :[37m %-10d [33m＃防 御 力 :[37m %-10d [33m＃速 度 值 :[37m %-10d [31m│[m\n",
       d.attack,d.resist,d.speed);
       prints(buf);  
       sprintf(buf, 
       "[1;31m │[33m＃抗魔能力 :[37m %-10d [33m＃战斗技术 :[37m %-10d [33m＃魔法技术 :[37m %-10d [31m│[m\n",
       d.mresist,d.hskill,d.mskill);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m │[33m＃头部装备 :[37m %-10s [33m＃右手装备 :[37m %-10s [33m＃左手装备 :[37m %-10s [31m│[m\n",
       weaponhead[d.weaponhead],weaponrhand[d.weaponrhand],weaponlhand[d.weaponlhand]);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m │[33m＃身体装备 :[37m %-10s [33m＃脚部装备 :[37m %-10s [33m            [37m            [31m│[m\n",
       weaponbody[d.weaponbody],weaponfoot[d.weaponfoot]);
       prints(buf);  
  
       sprintf(buf, 
       "[1;31m └————————————————————————————————————┘[m\n");
       prints(buf); 

       move(b_lines-1,0);
       sprintf(buf, 
       "                                                              [1;36m第二页[37m/[36m共二页[m\n");
       prints(buf);          
       break;
    }
    move(b_lines,0);
    sprintf(buf,"[1;44;37m  资料选单  [46m  [↑/PAGE UP]往上一页 [↓/PAGE DOWN]往下一页 [Q]离开:            [m");
    prints(buf);    
    pipkey=egetch();
    switch(pipkey)
    {
      case KEY_UP:
      case KEY_PGUP:
      case KEY_DOWN:
      case KEY_PGDN:
        if(page==1)
           page=2;
        else if(page==2)
           page=1;
        break;
#ifdef MAPLE
      case Ctrl('R'):
        if (currutmp->msgs[0].last_pid)
        {
          show_last_call_in();
          my_write(currutmp->msgs[0].last_pid, "水球丢回去：");
        }
        break;
#endif  // END MAPLE
    }
  }while((pipkey!='Q')&&(pipkey!='q')&&(pipkey!=KEY_LEFT));
  return 0;
}
