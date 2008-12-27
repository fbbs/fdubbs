/*---------------------------------------------------------------------------*/
/* 结局函式                                                                  */
/*                                                                           */
/*---------------------------------------------------------------------------*/
#include <time.h>
#include "bbs.h"
#include "pip.h"
extern struct chicken d;
extern time_t start_time;
extern time_t lasttime;
//#define getdata(a, b, c , d, e, f, g) getdata(a,b,c,d,e,f,NULL,g)

/*--------------------------------------------------------------------------*/
/*  结局参数设定                                                            */
/*--------------------------------------------------------------------------*/
struct newendingset
{
  char *girl;		/*女生结局的职业*/ 
  char *boy;		/*男生结局的职业*/
  int grade;		/*评分*/
};
typedef struct newendingset newendingset;

/*万能*/
struct newendingset endmodeall_purpose[] = {
"女性职业",		"男生职业",		0,
"成为这个国家新女王",	"成为这个国家新国王",	500,
"成为国家的宰相",	"成为国家的宰相",	400,
"成为教会中的大主教",	"成为教会中的大主教",	350,
"成为国家的大臣",	"成为国家的大臣",	320,
"成为一位博士",		"成为一位博士",		300,
"成为教会中的修女",	"成为教会中的神父",	150,
"成为法庭上的大法官",   "成为法庭上的法官",	200,
"成为知名的学者",	"成为知名的学者",	120,
"成为一名女官",		"成为一名男官",		100,
"在育幼院工作",		"在育幼院工作",		100,
"在旅馆工作",		"在旅馆工作",		100,
"在农场工作",		"在农场工作",		100,
"在餐厅工作",		"在餐厅工作",		100,
"在教堂工作",		"在教堂工作",		100,
"在地摊工作",		"在地摊工作",		100,
"在伐木场工作",		"在伐木场工作",		100,
"在美容院工作",		"在美容院工作",		100,
"在狩猎区工作",		"在狩猎区工作",		100,
"在工地工作",		"在工地工作",		100,
"在墓园工作",		"在墓园工作",		100,
"担任家庭教师工作",	"担任家庭教师工作",	100,
"在酒家工作",		"在酒家工作",		100,
"在酒店工作",		"在酒店工作",		100,
"在大夜总会工作",	"在大夜总会工作",	100,
"在家中帮忙",		"在家中帮忙",		50,
"在育幼院兼差",		"在育幼院兼差",		50,
"在旅馆兼差",		"在旅馆兼差",		50,
"在农场兼差",		"在农场兼差",		50,
"在餐厅兼差",		"在餐厅兼差",		50,
"在教堂兼差",		"在教堂兼差",		50,
"在地摊兼差",		"在地摊兼差",		50,
"在伐木场兼差",		"在伐木场兼差",		50,
"在美容院兼差",		"在美容院兼差",		50,
"在狩猎区兼差",		"在狩猎区兼差",		50,
"在工地兼差",		"在工地兼差",		50,
"在墓园兼差",		"在墓园兼差",		50,
"担任家庭教师兼差",	"担任家庭教师兼差",	50,
"在酒家兼差",		"在酒家兼差",		50,
"在酒店兼差",		"在酒店兼差",		50,
"在大夜总会兼差",	"在大夜总会兼差",	50,
NULL,		NULL,	0
};

/*战斗*/
struct newendingset endmodecombat[] = {
"女性职业",		"男生职业",			0,
"被封为勇者 战士型",  	"被封为勇者 战士型",		420,
"被拔擢成为一国的将军",	"被拔擢成为一国的将军",		300,
"当上了国家近卫队队长",	"当上了国家近卫队队长",		200,
"当了武术老师",		"当了武术老师",			150,
"变成骑士报效国家",	"变成骑士报效国家",		160,
"投身军旅生活，成为士兵","投身军旅生活，成为士兵",	80,
"变成奖金猎人",		"变成奖金猎人",			0,
"以  兵工作维生",	"以  兵工作维生",		0,
NULL,           NULL,   0
};

/*魔法*/
struct newendingset endmodemagic[] = {
"女性职业",	     	"男生职业",		0,
"被封为勇者 魔法型",	"被封为勇者 魔法型",	420,
"被聘为王宫魔法师",	"被聘为王官魔法师",	280,
"当了魔法老师",		"当了魔法老师",		160,
"变成一位魔导士",	"变成一位魔导士",	180,
"当了魔法师",		"当了魔法师",		120,
"以占卜师帮人算命为生",	"以占卜师帮人算命为生",	40,
"成为一个魔术师",	"成为一个魔术师",	20,
"成为街头艺人",		"成为街头艺人",		10,
NULL,           NULL	,0
};

/*社交*/
struct newendingset endmodesocial[] = {
"女性职业",     	"男生职业",		0,
"成为国王的宠妃",	"成为女王的驸马爷",	170,
"被挑选成为王妃",	"被选中当女王的夫婿",	260,
"被伯爵看中，成为夫人",	"成为了女伯爵的夫婿",	130,
"成为富豪的妻子",	"成为女富豪的夫婿",	100,
"成为商人的妻子",	"成为女商人的夫婿",	80,
"成为农人的妻子",	"成为女农人的夫婿",	80,
"成为地主的情妇",	"成为女地主的情夫",	-40,
NULL,           NULL,	0
};
/*艺术*/
struct newendingset endmodeart[] = {
"女性职业",		"男生职业",	0,
"成为了小丑",		"成为了小丑",	100,
"成为了作家",		"成为了作家",	100,
"成为了画家",		"成为了画家",	100,
"成为了舞蹈家",		"成为了舞蹈家",	100,
NULL,           NULL,	0
};

/*暗黑*/
struct newendingset endmodeblack[] = {
"女性职业",     	"男生职业",		0,
"变成了魔王",		"变成了魔王",		-1000,
"混成了太妹",		"混成了流氓",		-350,
"做了[ＳＭ女王]的工作",	"做了[ＳＭ国王]的工作",	-150,
"当了黑街的大姐",	"当了黑街的老大",	-500,
"变成高级娼妇",		"变成高级情夫",		-350,
"变成诈欺师诈欺别人",	"变成金光党骗别人钱",	-350,
"以流莺的工作生活",	"以牛郎的工作生活",	-350,
NULL,		NULL,	0
};

/*家事*/
struct newendingset endmodefamily[] = {
"女性职业",     	"男生职业",		0,
"正在新娘修行",		"正在新郎修行",		50,
NULL,		NULL,	0
};


int /*结局画面*/
pip_ending_screen()
{
  time_t now;
  char buf[256];
  char endbuf1[50];
  char endbuf2[50];
  char endbuf3[50];
  int endgrade=0;
  int endmode=0;
  clear();
  pip_ending_decide(endbuf1,endbuf2,endbuf3,&endmode,&endgrade);
  move(1,9); 
  prints("[1;33m┏━━━┓┏━━  ┓┏━━━  ┏━━━┓┏━━  ┓  ━━━  [0m");
  move(2,9);
  prints("[1;37m┃      ┃┃    ┃┃┃      ┃┃      ┃┃    ┃┃┃      ┃[0m");
  move(3,9);
  prints("[0;37m┃    ━  ┃    ┃┃┃      ┃┗━┓┏┛┃    ┃┃┃  ┏━┓[0m");
  move(4,9);
  prints("[0;37m┃    ━  ┃  ┃  ┃┃      ┃┏━┛┗┓┃  ┃  ┃┃      ┃[0m");
  move(5,9);
  prints("[1;37m┃      ┃┃  ┃  ┃┃      ┃┃      ┃┃  ┃  ┃┃      ┃[0m");
  move(6,9);
  prints("[1;35m┗━━━┛┗━  ━┛┗━━━  ┗━━━┛┗━  ━┛  ━━━  [0m");
  move(7,8);
  prints("[1;31m——————————[41;37m 星空战斗鸡结局报告 [0;1;31m———————————[0m");
  move(9,10);
  prints("[1;36m这个时间不知不觉地还是到临了...[0m");
  move(11,10);
  prints("[1;37m[33m%s[37m 得离开你的温暖怀抱，自己一只鸡在外面求生存了.....[0m",d.name);
  move(13,10);
  prints("[1;36m在你照顾教导他的这段时光，让他接触了很多领域，培养了很多的能力....[0m");
  move(15,10);
  prints("[1;37m因为这些，让小鸡 [33m%s[37m 之後的生活，变得更多采多姿了........[0m",d.name);
  move(17,10);
  prints("[1;36m对於你的关心，你的付出，你所有的爱......[0m");
  move(19,10);
  prints("[1;37m[33m%s[37m 会永远都铭记在心的....[0m",d.name);
  pressanykey("接下来看未来发展");
  clrchyiuan(7,19);
  move(7,8);
  prints("[1;34m——————————[44;37m 星空战斗鸡未来发展 [0;1;34m———————————[0m");
  move(9,10);
  prints("[1;36m透过水晶球，让我们一起来看 [33m%s[36m 的未来发展吧.....[0m",d.name);
  move(11,10);
  prints("[1;37m小鸡 [33m%s[37m 後来%s....[0m",d.name,endbuf1);
  move(13,10);
  prints("[1;36m因为他的之前的努力，使得他在这一方面%s....[0m",endbuf2);
  move(15,10);
  prints("[1;37m至於小鸡的婚姻状况，他後来%s，婚姻算是很美满.....[0m",endbuf3);
  move(17,10);
  prints("[1;36m嗯..这是一个不错的结局唷..........[0m");
  pressanykey("我想  你一定很感动吧.....");
  show_ending_pic(0);
  pressanykey("看一看分数罗");
  endgrade=pip_game_over(endgrade);
  pressanykey("下一页是小鸡资料  赶快copy下来做纪念");
  pip_data_list();
  pressanykey("欢迎再来挑战....");
  /*记录开始*/
  now=time(0);
  sprintf(buf, "[1;35m———————————————————————————————————————[0m\n");
  pip_log_record(buf);
  sprintf(buf, "[1;37m在 [33m%s [37m的时候，[36m%s [37m的小鸡 [32m%s[37m 出现了结局[0m\n", Cdate(&now), cuser.userid,d.name);
  pip_log_record(buf);
  sprintf(buf, "[1;37m小鸡 [32m%s [37m努力加强自己，後来%s[0m\n[1;37m因为之前的努力，使得在这一方面%s[0m\n",d.name,endbuf1,endbuf2);
  pip_log_record(buf);
  sprintf(buf, "[1;37m至於婚姻状况，他後来%s，婚姻算是很美满.....[0m\n\n[1;37m小鸡 [32n%s[37m 的总积分＝ [33m%d[0m\n",endbuf3,d.name,endgrade);
  pip_log_record(buf);
  sprintf(buf, "[1;35m———————————————————————————————————————[0m\n");
  pip_log_record(buf);  
  /*记录终止*/
  d.death=3;
  pipdie("[1;31m游戏结束罗...[m  ",3);
  return 0;
}

int 
pip_ending_decide(endbuf1,endbuf2,endbuf3,endmode,endgrade)
char *endbuf1,*endbuf2,*endbuf3;
int *endmode,*endgrade;
{
  char *name[8][2]={{"男的","女的"},
  	           {"嫁给王子","娶了公主"},
  	           {"嫁给你","娶了你"},
                   {"嫁给商人Ａ","娶了女商人Ａ"},
                   {"嫁给商人Ｂ","娶了女商人Ｂ"},
                   {"嫁给商人Ｃ","娶了女商人Ｃ"},
                   {"嫁给商人Ｄ","娶了女商人Ｄ"},
                   {"嫁给商人Ｅ","娶了女商人Ｅ"}}; 
  int m=0,n=0,grade=0;
  int modeall_purpose=0;
  char buf1[256];
  char buf2[256];
  
  *endmode=pip_future_decide(&modeall_purpose);
  switch(*endmode)
  {
  /*1:暗黑 2:艺术 3:万能 4:战士 5:魔法 6:社交 7:家事*/
    case 1:
      pip_endingblack(buf1,&m,&n,&grade);    
      break;
    case 2:
      pip_endingart(buf1,&m,&n,&grade);
      break;
    case 3:
      pip_endingall_purpose(buf1,&m,&n,&grade,modeall_purpose);
      break;
    case 4:
      pip_endingcombat(buf1,&m,&n,&grade);
      break;
    case 5:
      pip_endingmagic(buf1,&m,&n,&grade);
      break;
    case 6:
      pip_endingsocial(buf1,&m,&n,&grade);
      break;
    case 7:
      pip_endingfamily(buf1,&m,&n,&grade);
      break;
  }
  
  grade+=pip_marry_decide();
  strcpy(endbuf1, buf1);  
  if(n==1)
  {
    *endgrade=grade+300;
    sprintf(buf2,"非常的顺利..");
  }
  else if(n==2)
  {
    *endgrade=grade+100;
    sprintf(buf2,"表现还不错..");
  }
  else if(n==3)
  {
    *endgrade=grade-10;
    sprintf(buf2,"常遇到很多问题....");
  }  
  strcpy(endbuf2, buf2);  
  if(d.lover>=1 && d.lover <=7)
  {
    if(d.sex==1)
      sprintf(buf2,"%s",name[d.lover][1]);
    else
      sprintf(buf2,"%s",name[d.lover][0]);
  }
  else if(d.lover==10)
    sprintf(buf2,"%s",buf1);
  else if(d.lover==0)
  {
    if(d.sex==1)
      sprintf(buf2,"娶了同行的女孩");
    else
      sprintf(buf2,"嫁给了同行的男生");  
  } 
  strcpy(endbuf3, buf2);  
  return 0;
}
/*结局判断*/
/*1:暗黑 2:艺术 3:万能 4:战士 5:魔法 6:社交 7:家事*/
int
pip_future_decide(modeall_purpose)
int *modeall_purpose;
{
  int endmode;
  /*暗黑*/
  if((d.etchics==0 && d.offense >=100) || (d.etchics>0 && d.etchics<50 && d.offense >=250))
     endmode=1;
  /*艺术*/
  else if(d.art>d.hexp && d.art>d.mexp && d.art>d.hskill && d.art>d.mskill &&
          d.art>d.social && d.art>d.family && d.art>d.homework && d.art>d.wisdom &&
          d.art>d.charm && d.art>d.belief && d.art>d.manners && d.art>d.speech &&
          d.art>d.cookskill && d.art>d.love)
     endmode=2;
  /*战斗*/
  else if(d.hexp>=d.social && d.hexp>=d.mexp && d.hexp>=d.family)
  {
     *modeall_purpose=1;
     if(d.hexp>d.social+50 || d.hexp>d.mexp+50 || d.hexp>d.family+50)
        endmode=4;
     else
        endmode=3;     
  }
  /*魔法*/
  else if(d.mexp>=d.hexp && d.mexp>=d.social && d.mexp>=d.family)
  {  
     *modeall_purpose=2;
     if(d.mexp>d.hexp || d.mexp>d.social || d.mexp>d.family)
        endmode=5;
     else
        endmode=3;
  }   
  else if(d.social>=d.hexp && d.social>=d.mexp && d.social>=d.family)
  {
     *modeall_purpose=3;
     if(d.social>d.hexp+50 || d.social>d.mexp+50 || d.social>d.family+50)
        endmode=6;
     else
        endmode=3;
  }

  else 
  {  
     *modeall_purpose=4;
     if(d.family>d.hexp+50 || d.family>d.mexp+50 || d.family>d.social+50)
        endmode=7;
     else
        endmode=3;     
  }     
  return endmode;
}
/*结婚的判断*/
int
pip_marry_decide()
{
  int grade;
  if(d.lover!=0)
  {  
     /* 3 4 5 6 7:商人 */
     d.lover=d.lover;
     grade=80;
  }
  else
  {
     if(d.royalJ>=d.relation && d.royalJ>=100)
     {
        d.lover=1;  /*王子*/
        grade=200;
     }
     else if(d.relation>d.royalJ && d.relation>=100)
     {
        d.lover=2;  /*父亲或母亲*/
        grade=0;
     }
     else
     {
        d.lover=0;
        grade=40;
     }
  }
  return grade;
}


int
pip_endingblack(buf,m,n,grade) /*暗黑*/
char *buf;
int *m,*n,*grade;
{
 if(d.offense>=500 && d.mexp>=500) /*魔王*/
 {
   *m=1;
   if(d.mexp>=1000)
     *n=1;
   else if(d.mexp<1000 && d.mexp >=800)
     *n=2;
   else
     *n=3;
 }

else if(d.hexp>=600)  /*流氓*/
 {
   *m=2;
   if(d.wisdom>=350)
     *n=1;
   else if(d.wisdom<350 && d.wisdom>=300)
     *n=2;
   else 
     *n=3;  
 } 
 else if(d.speech>=100 && d.art>=80) /*SM*/
 {
   *m=3;
   if(d.speech>150 && d.art>=120)
     *n=1;
   else if(d.speech>120 && d.art>=100)
     *n=2;
   else   
     *n=3;
 }
 else if(d.hexp>=320 && d.character>200 && d.charm< 200)	/*黑街老大*/
 {
   *m=4;
   if(d.hexp>=400)
     *n=1;
   else if(d.hexp<400 && d.hexp>=360)
     *n=2;
   else 
     *n=3;  
 }
 else if(d.character>=200 && d.charm >=200 && d.speech>70 && d.toman >70)  /*高级娼妇*/
 {
   *m=5;
   if(d.charm>=300)
     *n=1;
   else if(d.charm<300 && d.charm>=250)
     *n=2;
   else 
     *n=3;  
 }
 
 else if(d.wisdom>=450)  /*诈骗师*/
 {
   *m=6;
   if(d.wisdom>=550)
     *n=1;
   else if(d.wisdom<550 && d.wisdom>=500)
     *n=2;
   else 
     *n=3;  
 }
 
 else /*流莺*/
 {
   *m=7;
   if(d.charm>=350)
     *n=1;
   else if(d.charm<350 && d.charm>=300)
     *n=2;
   else 
     *n=3;  
 }
 if(d.sex==1)
   strcpy(buf, endmodeblack[*m].boy);
 else
   strcpy(buf, endmodeblack[*m].girl);
 *grade=endmodeblack[*m].grade;
 return 0; 
}


int
pip_endingsocial(buf,m,n,grade) /*社交*/
char *buf;
int *m,*n,*grade;
{
 int class;
 if(d.social>600) class=1;
 else if(d.social>450) class=2;
 else if(d.social>380) class=3;
 else if(d.social>250) class=4;
 else class=5;

 switch(class)
 {
   case 1:
     if(d.charm>500)
     {
       *m=1;
       d.lover=10;
       if(d.character>=700)
        *n=1;
       else if(d.character<700 && d.character>=500)
        *n=2;
       else   
        *n=3;
     }
     else
     {
       *m=2;
       d.lover=10;
       if(d.character>=700)
        *n=1;
       else if(d.character<700 && d.character>=500)
        *n=2;
       else   
        *n=3;
     }
     break;
     
   case 2:
     *m=1;
     d.lover=10;
     if(d.character>=700)
        *n=1;
     else if(d.character<700 && d.character>=500)
        *n=2;
     else   
        *n=3;   
     break;
     
   case 3:
     if(d.character>=d.charm)
     {
       *m=3;
       d.lover=10;
       if(d.toman>=250)
         *n=1;
       else if(d.toman<250 && d.toman>=200)
         *n=2;
       else   
         *n=3;     
     }
     else
     {
       *m=4;
       d.lover=10;
       if(d.character>=400)
         *n=1;
       else if(d.character<400 && d.character>=300)
         *n=2;
       else   
         *n=3;     
     }
     break;
     
   case 4:
     if(d.wisdom>=d.affect)	
     {
	   *m=5;
	   d.lover=10;
	   if(d.toman>120 && d.cookskill>300 && d.homework>300)
	     *n=1;
	   else if(d.toman<120 && d.cookskill<300 && d.homework<300 &&d.toman>100 && d.cookskill>250 && d.homework>250)
	     *n=2;
	   else   
	     *n=3;     	
     }
     else
     {
	   *m=6;
	   d.lover=10;
	   if(d.hp>=400)
	     *n=1;
	   else if(d.hp<400 && d.hp>=300)
	     *n=2;
	   else   
	     *n=3;     
     }
     break;
   
   case 5:
     *m=7;
     d.lover=10;
     if(d.charm>=200)
       *n=1;
     else if(d.charm<200 && d.charm>=100)
       *n=2;
     else   
       *n=3;
     break;
 }
 if(d.sex==1)
   strcpy(buf, endmodesocial[*m].boy);
 else
   strcpy(buf, endmodesocial[*m].girl);
 *grade=endmodesocial[*m].grade;
 return 0; 
}

int
pip_endingmagic(buf,m,n,grade) /*魔法*/
char *buf;
int *m,*n,*grade;
{
 int class;
 if(d.mexp>800) class=1;
 else if(d.mexp>600) class=2;
 else if(d.mexp>500) class=3;
 else if(d.mexp>300) class=4;
 else class=5;

 switch(class)
 {
    case 1:
      if(d.affect>d.wisdom && d.affect>d.belief && d.etchics>100)
      {
	   *m=1;
	   if(d.etchics>=800)
	     *n=1;
	   else if(d.etchics<800 && d.etchics>=400)
	     *n=2;
	   else   
	     *n=3;      
      }
      else if(d.etchics<50)
      {
	   *m=4;
	   if(d.hp>=400)
	     *n=1;
	   else if(d.hp<400 && d.hp>=200)
	     *n=2;
	   else   
	     *n=3;    
      }
      else
      {
	   *m=2;
	   if(d.wisdom>=800)
	     *n=1;
	   else if(d.wisdom<800 && d.wisdom>=400)
	     *n=2;
	   else   
	     *n=3;      
      }
      break;
      
    case 2:
      if(d.etchics>=50)
      {
	   *m=3;
	   if(d.wisdom>=500)
	     *n=1;
	   else if(d.wisdom<500 && d.wisdom>=200)
	     *n=2;
	   else   
	     *n=3;     
      }
      else
      {
	   *m=4;
	   if(d.hp>=400)
	     *n=1;
	   else if(d.hp<400 && d.hp>=200)
	     *n=2;
	   else   
	     *n=3;          
      }
      break;
    
    case 3:
      *m=5;
      if(d.mskill>=300)
	*n=1;
      else if(d.mskill<300 && d.mskill>=150)
	*n=2;
      else   
	*n=3;
      break;

    case 4:
      *m=6;
      if(d.speech>=150)
	*n=1;
      else if(d.speech<150 && d.speech>=60)
	*n=2;
      else   
	*n=3;
      break; 
     
    case 5:
      if(d.character>=200)
      {
	*m=7;
        if(d.speech>=150)
 	  *n=1;
        else if(d.speech<150 && d.speech>=60)
      	  *n=2;
        else   
	  *n=3;        
      }
      else
      {
        *m=8;
        if(d.speech>=150)
 	  *n=1;
        else if(d.speech<150 && d.speech>=60)
      	  *n=2;
        else   
	  *n=3;      
      }
      break;
    
 }

 if(d.sex==1)
   strcpy(buf, endmodemagic[*m].boy);
 else
   strcpy(buf, endmodemagic[*m].girl); 
 *grade=endmodemagic[*m].grade;
 return 0; 
}

int
pip_endingcombat(buf,m,n,grade) /*战斗*/
char *buf;
int *m,*n,*grade;
{
 int class;
 if(d.hexp>1500) class=1;
 else if(d.hexp>1000) class=2;
 else if(d.hexp>800) class=3;
 else class=4;

 switch(class)
 {
    case 1:
      if(d.affect>d.wisdom && d.affect>d.belief && d.etchics>100)
      {
	   *m=1;
	   if(d.etchics>=800)
	     *n=1;
	   else if(d.etchics<800 && d.etchics>=400)
	     *n=2;
	   else   
	     *n=3;      
      }
      else if(d.etchics<50)
      {

      }
      else
      {
	   *m=2;
	   if(d.wisdom>=800)
	     *n=1;
	   else if(d.wisdom<800 && d.wisdom>=400)
	     *n=2;
	   else   
	     *n=3;      
      }
      break;	
    
    case 2:
      if(d.character>=300 && d.etchics>50)
      {
	   *m=3;
	   if(d.etchics>=300 && d.charm >=300)
	     *n=1;
	   else if(d.etchics<300 && d.charm<300 && d.etchics>=250 && d.charm >=250)
	     *n=2;
	   else   
	     *n=3;      
      }
      else if(d.character<300 && d.etchics>50)
      {
	   *m=4;
	   if(d.speech>=200)
	     *n=1;
	   else if(d.speech<150 && d.speech>=80)
	     *n=2;
	   else   
	     *n=3;      
      }
      else
      {
	   *m=7;
	   if(d.hp>=400)
	     *n=1;
	   else if(d.hp<400 && d.hp>=200)
	     *n=2;
	   else   
	     *n=3;          
      }
      break;
    
    case 3:
      if(d.character>=400 && d.etchics>50)
      {
	   *m=5;
	   if(d.etchics>=300)
	     *n=1;
	   else if(d.etchics<300 && d.etchics>=150)
	     *n=2;
	   else   
	     *n=3;      
      }
      else if(d.character<400 && d.etchics>50)
      {
	   *m=4;
	   if(d.speech>=200)
	     *n=1;
	   else if(d.speech<150 && d.speech>=80)
	     *n=2;
	   else   
	     *n=3;      
      }
      else
      {
	   *m=7;
	   if(d.hp>=400)
	     *n=1;
	   else if(d.hp<400 && d.hp>=200)
	     *n=2;
	   else   
	     *n=3;          
      }
      break;    
    
    case 4:
      if(d.etchics>=50)
      {
	   *m=6;
      }
      else
      {
	   *m=8;
      }
      if(d.hskill>=100)
        *n=1;
      else if(d.hskill<100 && d.hskill>=80)
        *n=2;
      else   
        *n=3;       
      break;
 }

 if(d.sex==1)
   strcpy(buf, endmodecombat[*m].boy);
 else
   strcpy(buf, endmodecombat[*m].girl);
 *grade=endmodecombat[*m].grade;
 return 0;
}


int
pip_endingfamily(buf,m,n,grade) /*家事*/
char *buf;
int *m,*n,*grade;
{
 *m=1;
 if(d.charm>=200)
   *n=1;
 else if(d.charm<200 && d.charm>100)
   *n=2;
 else 
   *n=3;
   
 if(d.sex==1)
   strcpy(buf, endmodefamily[*m].boy);
 else
   strcpy(buf, endmodefamily[*m].girl);
 *grade=endmodefamily[*m].grade;
 return 0;
}


int
pip_endingall_purpose(buf,m,n,grade,mode) /*万能*/
char *buf;
int *m,*n,*grade;
int mode;
{
 int data;
 int class;
 int num=0;
 
 if(mode==1)
    data=d.hexp;
 else if(mode==2)
    data=d.mexp;
 else if(mode==3)
    data=d.social;
 else if(mode==4)
    data=d.family;
 if(class>1000) class=1;
 else if(class>800) class=2;
 else if(class>500) class=3;
 else if(class>300) class=4;
 else class=5;

 data=pip_max_worktime(&num);
 switch(class)
 {
   case 1:
	if(d.character>=1000)
	{
	   *m=1;
	   if(d.etchics>=900)
	     *n=1;
	   else if(d.etchics<900 && d.etchics>=600)
	     *n=2;
	   else   
	     *n=3;	
	}
	else
	{
	   *m=2;
	   if(d.etchics>=650)
	     *n=1;
	   else if(d.etchics<650 && d.etchics>=400)
	     *n=2;
	   else   
	     *n=3;	
	}
	break;
   
   case 2:
	if(d.belief > d.etchics && d.belief > d.wisdom)
	{
	   *m=3;
	   if(d.etchics>=500)
	     *n=1;
	   else if(d.etchics<500 && d.etchics>=250)
	     *n=2;
	   else   
	     *n=3;	
	}
	else if(d.etchics > d.belief && d.etchics > d.wisdom)
	{
	   *m=4;
	   if(d.wisdom>=800)
	     *n=1;
	   else if(d.wisdom<800 && d.wisdom>=600)
	     *n=2;
	   else   
	     *n=3;	
	}
	else
	{
	   *m=5;
	   if(d.affect>=800)
	     *n=1;
	   else if(d.affect<800 && d.affect>=400)
	     *n=2;
	   else   
	     *n=3;	
	}
	break;

   case 3:
	if(d.belief > d.etchics && d.belief > d.wisdom)
	{
	   *m=6;
	   if(d.belief>=400)
	     *n=1;
	   else if(d.belief<400 && d.belief>=150)
	     *n=2;
	   else   
	     *n=3;	
	}
	else if(d.etchics > d.belief && d.etchics > d.wisdom)
	{
	   *m=7;
	   if(d.wisdom>=700)
	     *n=1;
	   else if(d.wisdom<700 && d.wisdom>=400)
	     *n=2;
	   else   
	     *n=3;	
	}
	else
	{
	   *m=8;
	   if(d.affect>=800)
	     *n=1;
	   else if(d.affect<800 && d.affect>=400)
	     *n=2;
	   else   
	     *n=3;	
	}
	break;   

   case 4:
	if(num>=2)
	{
	   *m=8+num;
	   switch(num)
	   {
	   	case 2:
	   		if(d.love>100)	*n=1;
	   		else if(d.love>50) *n=2;
	   		else *n=3;
	   		break;
	   	case 3:
	   		if(d.homework>100) *n=1;
	   		else if(d.homework>50) *n=2;
	   		else *n=3;
	   		break;
	   	case 4:
	   		if(d.hp>600) *n=1;
	   		else if(d.hp>300) *n=2;
	   		else *n=3;
	   		break;
	   	case 5:
	   		if(d.cookskill>200) *n=1;
	   		else if(d.cookskill>100) *n=2;
	   		else *n=3;
	   		break;
	   	case 6:
	   		if((d.belief+d.etchics)>600) *n=1;
	   		else if((d.belief+d.etchics)>200) *n=2;
	   		else *n=3;
	   		break;
	   	case 7:
	   		if(d.speech>150) *n=1;
	   		else if(d.speech>50) *n=2;
	   		else *n=3;
	   		break;
	   	case 8:
	   		if((d.hp+d.wrist)>900) *n=1;
	   		else if((d.hp+d.wrist)>600) *n=2;
	   		else *n=3;
	   		break;
	   	case 9:
	   	case 11:
	   		if(d.art>250) *n=1;
	   		else if(d.art>100) *n=2;
	   		else *n=3;
	   		break;
	   	case 10:
	   		if(d.hskill>250) *n=1;
	   		else if(d.hskill>100) *n=2;
	   		else *n=3;
	   		break;
	   	case 12:
	   		if(d.belief>500) *n=1;
	   		else if(d.belief>200) *n=2;
	   		else *n=3;
	   		break;
	   	case 13:
	   		if(d.wisdom>500) *n=1;
	   		else if(d.wisdom>200) *n=2;
	   		else *n=3;
	   		break;	
	   	case 14:
	   	case 16:
	   		if(d.charm>1000) *n=1;
	   		else if(d.charm>500) *n=2;
	   		else *n=3;
	   		break;
	   	case 15:
	   		if(d.charm>700) *n=1;
	   		else if(d.charm>300) *n=2;
	   		else *n=3;
	   		break;	   	
	   }
	}
	else
	{
	   *m=9;
	   if(d.etchics > 400)
	     *n=1;
	   else if(d.etchics >200)
	     *n=2;
	   else
	     *n=3;
	}
	break;
   case 5:
	if(num>=2)
	{
	   *m=24+num;
	   switch(num)
	   {
	   	case 2:
	   	case 3:
	   		if(d.hp>400) *n=1;
	   		else if(d.hp>150) *n=2;
	   		else *n=3;
	   		break;
	   	case 4:
	   	case 10:
	   	case 11:
	   		if(d.hp>600) *n=1;
	   		else if(d.hp>300) *n=2;
	   		else *n=3;
	   		break;
	   	case 5:
	   		if(d.cookskill>150) *n=1;
	   		else if(d.cookskill>80) *n=2;
	   		else *n=3;
	   		break;
	   	case 6:
	   		if((d.belief+d.etchics)>600) *n=1;
	   		else if((d.belief+d.etchics)>200) *n=2;
	   		else *n=3;
	   		break;
	   	case 7:
	   		if(d.speech>150) *n=1;
	   		else if(d.speech>50) *n=2;
	   		else *n=3;
	   		break;
	   	case 8:
	   		if((d.hp+d.wrist)>700) *n=1;
	   		else if((d.hp+d.wrist)>300) *n=2;
	   		else *n=3;
	   		break;
	   	case 9:
	   		if(d.art>100) *n=1;
	   		else if(d.art>50) *n=2;
	   		else *n=3;
	   		break;
	   	case 12:
	   		if(d.hp>300) *n=1;
	   		else if(d.hp>150) *n=2;
	   		else *n=3;
	   		break;
	   	case 13:
	   		if(d.speech>100) *n=1;
	   		else if(d.speech>40) *n=2;
	   		else *n=3;
	   		break;	
	   	case 14:
	   	case 16:
	   		if(d.charm>1000) *n=1;
	   		else if(d.charm>500) *n=2;
	   		else *n=3;
	   		break;
	   	case 15:
	   		if(d.charm>700) *n=1;
	   		else if(d.charm>300) *n=2;
	   		else *n=3;
	   		break;	   	
	   }
	}
	else
	{
	   *m=25;
	   if(d.relation > 100)
	     *n=1;
	   else if(d.relation >50)
	     *n=2;
	   else
	     *n=3;
	}
	break; 
 } 

 if(d.sex==1)
   strcpy(buf, endmodeall_purpose[*m].boy);
 else
   strcpy(buf, endmodeall_purpose[*m].girl);
 *grade=endmodeall_purpose[*m].grade;
 return 0;
}

int
pip_endingart(buf,m,n,grade) /*艺术*/
char *buf;
int *m,*n,*grade;
{
 if(d.speech>=100)
 {
   *m=1;
   if(d.hp>=300 && d.affect>=350)
     *n=1;
   else if(d.hp<300 && d.affect<350 && d.hp>=250 && d.affect>=300)
     *n=2;
   else   
     *n=3;
 } 
 else if(d.wisdom>=400)
 {
   *m=2;
   if(d.affect>=500)
     *n=1;
   else if(d.affect<500 && d.affect>=450)
     *n=2;
   else   
     *n=3;
 }
 else if(d.classI>=d.classJ)
 {
   *m=3;
   if(d.affect>=350)
     *n=1;
   else if(d.affect<350 && d.affect>=300)
     *n=2;
   else   
     *n=3;
 }
 else 
 {
   *m=4;
   if(d.affect>=200 && d.hp>150)
     *n=1;
   else if(d.affect<200 && d.affect>=180 && d.hp>150)
     *n=2;
   else   
     *n=3;
 }
 if(d.sex==1)
   strcpy(buf, endmodeart[*m].boy);
 else
   strcpy(buf, endmodeart[*m].girl);
 *grade=endmodeart[*m].grade;  
 return 0;
}

int
pip_max_worktime(num)
int *num;
{
  int data=20;
  if(d.workA>data)
  {
     data=d.workA;
     *num=1;
  }
  if(d.workB>data)
  {
     data=d.workB;
     *num=2;
  }
  if(d.workC>data)
  {
     data=d.workC;
     *num=3;
  }
  if(d.workD>data)
  {
     data=d.workD;
     *num=4;
  }
  if(d.workE>data)
  {
     data=d.workE;
     *num=5;
  }

  if(d.workF>data)
  {
     data=d.workF;
     *num=6;
  }
  if(d.workG>data)
  {
     data=d.workG;
     *num=7;
  }
  if(d.workH>data)
  {
     data=d.workH;
     *num=8;
  }
  if(d.workI>data)
  {
     data=d.workI;
     *num=9;
  }
  if(d.workJ>data)
  {
     data=d.workJ;
     *num=10;
  }
  if(d.workK>data)
  {
     data=d.workK;
     *num=11;
  }
  if(d.workL>data)
  {
     data=d.workL;
     *num=12;
  }
  if(d.workM>data)
  {
     data=d.workM;
     *num=13;
  }
  if(d.workN>data)
  {
     data=d.workN;
     *num=14;
  }
  if(d.workO>data)
  {
     data=d.workO;
     *num=16;
  }
  if(d.workP>data)
  {
     data=d.workP;
     *num=16;
  }  

  return data;
}

int pip_game_over(endgrade)
int endgrade;
{
	long gradebasic;
	long gradeall;
	
	gradebasic=(d.maxhp+d.wrist+d.wisdom+d.character+d.charm+d.etchics+d.belief+d.affect)/10-d.offense;
	clrchyiuan(1,23);
	gradeall=gradebasic+endgrade;
	move(8,17);
	prints("[1;36m感谢您玩完整个星空小鸡的游戏.....[0m");
	move(10,17);
	prints("[1;37m经过系统计算的结果：[0m");
	move(12,17);
	prints("[1;36m您的小鸡 [37m%s [36m总得分＝ [1;5;33m%d [0m",d.name,gradeall);
	return gradeall;
}

int pip_divine() /*占卜师来访*/
{
  char buf[256];
  char ans[4];
  char endbuf1[50];
  char endbuf2[50];
  char endbuf3[50];
  int endgrade=0;
  int endmode=0;
  long money;
  int tm;
  int randvalue;
  
  tm=d.bbtime/60/30;
  move(b_lines-2,0);
  money=300*(tm+1);
  clrchyiuan(0,24);
  move(10,14);
  prints("[1;33;5m叩叩叩...[0;1;37m突然传来阵阵的敲门声.........[0m");
  pressanykey("去瞧瞧是谁吧......");
  clrchyiuan(0,24);
  move(10,14);
  prints("[1;37;46m    原来是云游四海的占卜师来访了.......    [0m");
  pressanykey("开门让他进来吧....");
  if(d.money>=money)
  {
    randvalue=rand()%5;
    sprintf(buf,"你要占卜吗? 要花%d元喔...[Y/n]",money);
#ifdef MAPLE
    getdata(12,14,buf, ans, 2, 1, 0);
#else
    getdata(12,14,buf, ans, 2, DOECHO, YEA);
#endif  // END MAPLE
    if(ans[0]!='N' && ans[0]!='n')
    {
      pip_ending_decide(endbuf1,endbuf2,endbuf3,&endmode,&endgrade);
      if(randvalue==0)
      		sprintf(buf,"[1;37m  你的小鸡%s以後可能的身份是%s  [0m",d.name,endmodemagic[2+rand()%5].girl);
      else if(randvalue==1)
      		sprintf(buf,"[1;37m  你的小鸡%s以後可能的身份是%s  [0m",d.name,endmodecombat[2+rand()%6].girl);
      else if(randvalue==2)
      		sprintf(buf,"[1;37m  你的小鸡%s以後可能的身份是%s  [0m",d.name,endmodeall_purpose[6+rand()%15].girl);
      else if(randvalue==3)
      		sprintf(buf,"[1;37m  你的小鸡%s以後可能的身份是%s  [0m",d.name,endmodeart[2+rand()%6].girl);
      else if(randvalue==4)
      		sprintf(buf,"[1;37m  你的小鸡%s以後可能的身份是%s  [0m",d.name,endbuf1);
      d.money-=money;
      clrchyiuan(0,24);
      move(10,14);
      prints("[1;33m在我占卜结果看来....[0m");
      move(12,14);
      prints(buf);
      pressanykey("谢谢惠顾，有缘再见面了.(不准不能怪我喔)");
    }
    else
    {
      pressanykey("你不想占卜啊?..真可惜..那只有等下次吧...");
    }
  }
  else
  {
    pressanykey("你的钱不够喔..真是可惜..等下次吧...");
  }
  return 0;
}
