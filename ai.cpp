/**********************************--日志--**************************************
版本信息:
v1.0
1. 基本功能实现, 仍需优化部分算法

v1.1
1. 增加命名空间, 具体效果尚未测试

预计增加及优化的内容:
1.优化Attackable_中对行进速度v、进攻能力o、防御能力o、回复速率v等影响的计算     o
2.增加Reattack_中的支援算法                                                     o
3.优化SearchBesAim_中对求援能力的、防御回复进攻能力的考虑                       v
4.优化Attack_中的围攻、支援算法                                                 o
5.优化Passable_中对回复速率及剩余兵力的考虑                                     v
6.215、216行有未知功能的代码                              代码已乱, 未表现出bug v
7.是否需要借助障碍物防御                                         似乎没有障碍物 x
8.优化CutLine_中进攻后切断相关                                                  o
9.在下图模式中惨败, 需分析原因                    具体原因未知, 增加G策略后完胜 v
 ---------------------------------
|../player_ai/ai/Release/ai.dll   |
|../sample_ai/sample_ai_ver1.3.dll|
|../sample_ai/random_ai_2.dll     |
|../sample_ai/sample_ai_ver1.1.dll|
 ---------------------------------

v1.2
1. 更新一些内容, 网页无法通过编译的现象仍然存在

预计增加及优化的内容:
1. 尽早找出网页无法编译通过的bug的原因                     已解决, 为网站的问题 v
2. 同步切断函数及延伸函数中一些参数的设定                                       v
3. 增加围攻功能                                                                 o
4. 优化进攻数目                                                                 o
5. 渔翁得利算法？                                                               o
6. 优化CutLine_中进攻后切断相关                                                 v
7. 在线比赛结果不尽如意, 仍需努力                                               o

v1.3
1. 更改Attactktowers的统计规则
2. 增加防御的优先级
3. 在猥琐发育中增加生长塔于势力防御之前 (在本地对决中有一定效果) v1.3.5
4. 增加快速攻击算法, 需完善 v1.3.6 v1.3.7 (存在逻辑bug)
5. 增加最后一击算法 (统计势力所拥有的塔存在bug?) v1.3.5
6. 增加塔的总资源信息
7. 手动抢占12号塔 v1.3.8
8. 优化大量算法及数据名称 v1.3.9

v1.4
1. 修复v1.3.7中发现的bug, 并优化算法

*********************************************************************************/

#include "ai.h"
#include "definition.h"
#include "user_toolbox.h"
#include <math.h>
#include <iostream>
#include <vector>
#include <time.h>
#include <stdlib.h>

namespace Alchemist {      //尝试使用命名空间解决无法在线游戏的问题
	/*数据类型定义*/
	typedef struct MyPlayerInfo {
		TPlayerID ID;
		double TResource;      //势力的总资源数
		vector <TTowerID> TowersID;      //对应着info.towerInfo和Tower中的顺序
	};
	typedef struct MyTowerInfo {
		TTowerID ID;
		int currLineNum;      //当前兵线数 = info.towerInfo[MYID].currLineNum
		TResourceD resourcesum;      //总资源
		TowerStrategy strategy;      //实时更新的塔属性
		vector <TTowerID> Aimtowers;      //正进攻的塔
		vector <TTowerID> Attactktowers;      //进攻该塔的塔
	};

	/*数据声明*/
	int step = 0;      //当前操作数，上限为info.myMaxControl
	Command C;      //命令结构体
	vector<MyPlayerInfo> Player;      //玩家, 我方为[0]
	vector<MyTowerInfo> Tower;      //塔,顺序对应info.towerInfo[]中顺序
	vector <TTowerID> Toweraims;      //进攻函数相关参数

	/*函数声明区*/
	void Initialize_(Info& info);      //信息初始化
	void HappyGrow_(Info& info);      //猥琐发育
	bool Passable_(Info& info, TTowerID tower1, TTowerID tower2);      //判断能否正常出兵1.0
	double Attackable_(Info& info, TTowerID tower1, TTowerID tower2);      //判断能否进攻1.0
	void Reattack_(Info& info, TTowerID tower1, TTowerID tower2);      //反击函数
	void Attack_(Info& info);      //进攻函数
	void SearchBesAim_(Info& info, TTowerID tower);      //寻找最优进攻目标 
	void CutLine_(Info& info);      //切断兵线
	void QuiklyAttack_(Info& info);      //快速切断
	double vGrow_(Info& info, TTowerID tower);
	double vDown_(Info& info, TTowerID tower);
	double v_(Info& info, TTowerID tower);
}

using namespace Alchemist;

/*选手主函数*/

void player_ai(Info& info)
{
	Initialize_(info);

	//先发占领12号塔
	if (info.round <= 20 && info.towerInfo[Player[0].TowersID[2]].resource > 20
		&& Tower[12].Attactktowers.size() == 0) {
		info.myCommandList.addCommand(addLine, Player[0].TowersID[2], 12);
		step++;
	}

	QuiklyAttack_(info);

	for (int i = 0; i < Player[0].TowersID.size(); i++) {      //触发反击
		if (step >= info.myMaxControl)
			break;
		for (int j = 0; j < Tower[Player[0].TowersID[i]].Attactktowers.size(); j++)
			Reattack_(info, Player[0].TowersID[i], Tower[Player[0].TowersID[i]].Attactktowers[j]);
	}

	if (step < info.myMaxControl) {
		HappyGrow_(info);
	}

	if (step < info.myMaxControl && 
		info.round > 26 && (info.playerInfo[info.myID].towers.size() < info.towerNum - 1 || info.round < 240))
		CutLine_(info);
	else if (step < info.myMaxControl&&info.round == 25 && info.lineInfo[Player[0].TowersID[2]][12].exist) {
		info.myCommandList.addCommand(cutLine, Player[0].TowersID[2], 12, 1);
	}
	else if(step < info.myMaxControl) {
		TTowerID LonelyTown = Player[1].TowersID[0];
		for (int i = 0; i < info.playerInfo[info.myID].towers.size(); i++) {
			if (step >= info.myMaxControl)
				break;
			if (Passable_(info, Player[0].TowersID[i], LonelyTown)) {
				if (Tower[Player[0].TowersID[i]].strategy != Attack
					&&Tower[LonelyTown].strategy != Defence) {      //攻击模式
					info.myCommandList.addCommand(changeStrategy, Player[0].TowersID[i], Attack);
					Tower[Player[0].TowersID[i]].strategy = Attack;
					step++;
				}
				if (step < info.myMaxControl) {
					info.myCommandList.addCommand(addLine, Player[0].TowersID[i], LonelyTown);
					step++;
				}
			}
		}
	}


	if (info.towerInfo[Player[0].TowersID[0]].resource >= 60
		&& step < info.myMaxControl)
		Attack_(info);
}


/*函数定义区*/

//初始化信息
void Alchemist::Initialize_(Info& info) {
	step = 0;

	//初初始化Player[]信息以及势力ID改变时更新数据
	Player.clear();
	MyPlayerInfo IDtemp;
	IDtemp.ID = info.myID;
	IDtemp.TResource = 0;
	Player.push_back(IDtemp);
	for (int i = 0; i < info.playerSize; i++) {      //初始化ID数组, k为Player的序号
		if (info.playerInfo[i].id != Player[0].ID&&info.playerInfo[i].alive) {      //非我方势力
			MyPlayerInfo IDtemp;
			IDtemp.ID = i;
			IDtemp.TResource = 0;
			Player.push_back(IDtemp);
		}
	}

	//实时统计势力资源及塔信息
	Tower.clear();      //清空塔数组
	for (int i = 0; i < info.towerNum; i++) {      //统计势力的总资源
		for (int j = 0; j < Player.size(); j++) {
			if (info.towerInfo[i].owner == Player[j].ID) {
				Player[j].TResource += info.towerInfo[i].resource;
				Player[j].TowersID.push_back(i);
				break;
			}
		}
		MyTowerInfo temptower;
		temptower.ID = i;
		temptower.resourcesum = info.towerInfo[i].resource;
		temptower.strategy = info.towerInfo[i].strategy;
		for (int k = 0; k < info.towerNum; k++) {      //统计塔信息
			if (info.lineInfo[i][k].exist) {
				temptower.Aimtowers.push_back(k);
				if (info.lineInfo[i][k].resource != 0)
					temptower.resourcesum += info.lineInfo[i][k].resource;
				else if (info.lineInfo[i][k].backResource != 0)
					temptower.resourcesum += info.lineInfo[i][k].backResource;
			}
			if (info.lineInfo[k][i].exist && info.towerInfo[i].owner != info.towerInfo[k].owner)
				temptower.Attactktowers.push_back(k);
		}
		Tower.push_back(temptower);
	}

	//各方塔按总资源由大到小排序
	for (int k = 0; k < Player.size(); k++)
		for (int i = 1; i < Player[k].TowersID.size(); i++) 
			for (int j = i + 1; j < Player[k].TowersID.size(); j++) {
				if (Tower[Player[k].TowersID[i]].resourcesum < Tower[Player[k].TowersID[j]].resourcesum) {
					int temp = Player[k].TowersID[i];
					Player[k].TowersID[i] = Player[k].TowersID[j];
					Player[k].TowersID[j] = temp;
				}
			}
}

//猥琐发育
void Alchemist::HappyGrow_(Info& info) {
	//无脑势力属性提升
	if (info.playerInfo[Player[0].ID].technologyPoint >=      //资源再生
		RegenerationSpeedUpdateCost[info.playerInfo[Player[0].ID].RegenerationSpeedLevel]
		&& info.playerInfo[Player[0].ID].RegenerationSpeedLevel < MAX_REGENERATION_SPEED_LEVEL) {
		info.myCommandList.addCommand(upgrade, RegenerationSpeed);
		step++;
	}

	//改变塔策略为生长
	for (int i = 0; i < Player[0].TowersID.size(); i++) {
		if (step >= info.myMaxControl)
			return;
		if (info.towerInfo[Player[0].TowersID[i]].resource < info.towerInfo[Player[0].TowersID[i]].maxResource - 20)
			switch (Tower[Player[0].TowersID[i]].strategy)
			{
			case Normal:
			case Defence:
				if (Tower[Player[0].TowersID[i]].Attactktowers.size() == 0
					&& info.playerInfo[Player[0].ID].technologyPoint >=
					StrategyChangeCost[Tower[Player[0].TowersID[i]].strategy][Grow]) {
					info.myCommandList.addCommand(changeStrategy, Player[0].TowersID[i], Grow);
					Tower[Player[0].TowersID[i]].strategy = Grow;
					step++;
				}
				break;
			case Attack:
				if (Tower[Player[0].TowersID[i]].Aimtowers.size() == 0
					&& info.playerInfo[Player[0].ID].technologyPoint >= 5) {
					info.myCommandList.addCommand(changeStrategy, Player[0].TowersID[i], Grow);
					Tower[Player[0].TowersID[i]].strategy = Grow;
					step++;
				}
				break;
			default:
				break;
			}
	}

	if (info.playerInfo[Player[0].ID].technologyPoint >=      //防御能力
		DefenceStageUpdateCost[info.playerInfo[Player[0].ID].DefenceLevel]
		&& info.playerInfo[Player[0].ID].DefenceLevel < MAX_DEFENCE_LEVEL) {
		info.myCommandList.addCommand(upgrade, Wall);
		step++;
	}
	if (info.playerInfo[Player[0].ID].technologyPoint >=      //行动力
		DefenceStageUpdateCost[info.playerInfo[Player[0].ID].ExtraControlLevel]
		&& info.playerInfo[Player[0].ID].ExtraControlLevel < MAX_EXTRA_CONTROL_LEVEL) {
		info.myCommandList.addCommand(upgrade, ExtraControl);
		step++;
	}
	if (info.playerInfo[Player[0].ID].technologyPoint >=      //速度
		DefenceStageUpdateCost[info.playerInfo[Player[0].ID].ExtendingSpeedLevel]
		&& info.playerInfo[Player[0].ID].ExtendingSpeedLevel < MAX_EXTENDING_SPEED_LEVEL) {
		info.myCommandList.addCommand(upgrade, ExtendingSpeed);
		step++;
	}

	//无脑传输
	for (int i = 1; i < Player[0].TowersID.size(); i++) {
		if (step >= info.myMaxControl
			|| (Tower[i].Attactktowers.size() > 0      //受到攻击或主塔即将到达上限兵力数
			|| info.towerInfo[Player[0].TowersID[0]].maxResource - info.towerInfo[Player[0].TowersID[0]].resource < 15)
			)
			break;
		/*部分代码转移至CutLine_()*/
		else if (Attackable_(info, Player[0].TowersID[i], Player[0].TowersID[0]) > 40
			&& info.towerInfo[Player[0].TowersID[i]].resource >= 10      //初始值大约在10
			&& info.towerInfo[Player[0].TowersID[0]].resource < info.towerInfo[Player[0].TowersID[0]].maxResource) {
			if (Passable_(info, Player[0].TowersID[i], Player[0].TowersID[0])) {      //目标兵线不存在且兵力可到达
				info.myCommandList.addCommand(addLine, Player[0].TowersID[i], Player[0].TowersID[0]);
				step++;
			}
		}
	}
}

//判断能否正常出兵2.0
bool Alchemist::Passable_(Info& info, TTowerID tower1, TTowerID tower2) {
	//线路判断
	if (tower1 == tower2      //同一个塔
		|| info.mapInfo->passable(info.towerInfo[tower1].position,
			info.towerInfo[tower2].position) == false      //无路可走
		|| info.lineInfo[tower1][tower2].exist) {      //兵线存在
		return false;
	}

	double D = getDistance(info.towerInfo[tower1].position, info.towerInfo[tower2].position) / 10;     //计算距离

	//兵力判断
	if (info.towerInfo[tower1].currLineNum < info.towerInfo[tower1].maxLineNum      //可派兵
		&&info.towerInfo[tower1].resource >= D)      //兵力足够出征
		return true;
	else
		return false;
}

//判断能否进攻2.0
double Alchemist::Attackable_(Info& info, TTowerID tower1, TTowerID tower2) {
	if (Passable_(info, tower1, tower2) == false)      //不可出征
		return 0;

	double D = getDistance(info.towerInfo[tower1].position, info.towerInfo[tower2].position) / 10;     //计算距离
	double t = D / v_(info, tower1);      //到达需要的时间
	double S = info.towerInfo[tower1].resource + (vGrow_(info, tower1) * t - vDown_(info, tower1) * t);      //到达后的源
	double A = Tower[tower2].resourcesum + vGrow_(info, tower2) * t;      //到达后的目标

	//满足到达后的源大于到达后的目标+10
	if (S > D + A + 10)      //比较某时刻的兵力，需计算之后的兵力
		return S - D - A - 10;
	else
		return 0;
}

//反击函数及防御
void Alchemist::Reattack_(Info& info, TTowerID tower1, TTowerID tower2) {
	if (step < info.myMaxControl && Attackable_(info, tower1, tower2) == 0) {      //无法成功反击
		if (Tower[tower1].strategy != Defence
			&&info.towerInfo[tower2].strategy == Attack) {
			if (info.playerInfo[Player[0].ID].technologyPoint >= 5
				&& info.towerInfo[tower2].owner != info.myID) {
				info.myCommandList.addCommand(changeStrategy, tower1, Defence);
				Tower[tower1].strategy = Defence;
				step++;
			}
		}
		else if (Tower[tower1].strategy != Grow
			&&info.towerInfo[tower2].strategy == Normal)
			if (info.playerInfo[Player[0].ID].technologyPoint >= 5
				&& info.towerInfo[tower2].owner != info.myID) {
				info.myCommandList.addCommand(changeStrategy, tower1, Grow);
				Tower[tower1].strategy = Grow;
				step++;
			}
	}
	else if (step < info.myMaxControl
		&&info.towerInfo[tower2].owner != info.myID) {
		info.myCommandList.addCommand(addLine, tower1, tower2);
		step++;
	}
}

//进攻函数
void Alchemist::Attack_(Info& info) {
	for (int k = 0; k < Player[0].TowersID.size(); k++) {
		if (step >= info.myMaxControl)
			break;
		SearchBesAim_(info, Player[0].TowersID[k]);
		for (int i = 0; i < Toweraims.size(); i++) {
			if (step >= info.myMaxControl)
				break;
			else {
				if (Tower[Player[0].TowersID[k]].strategy != Attack
					&&Tower[Toweraims[i]].strategy != Defence) {      //攻击模式...加了这个后简直上天了...
					info.myCommandList.addCommand(changeStrategy, Player[0].TowersID[k], Attack);
					Tower[Player[0].TowersID[k]].strategy = Attack;
					step++;
				}
				if (step < info.myMaxControl) {
					info.myCommandList.addCommand(addLine, Player[0].TowersID[k], Toweraims[i]);
					step++;
				}
			}
		}
	}
}

//寻找最优进攻目标 
void Alchemist::SearchBesAim_(Info& info, TTowerID tower) {
	Toweraims.clear();
	vector <double> S;
	double Stemp;
	for (int i = 0; i < info.towerNum; i++) {      //统计可进攻塔的信息
		Stemp = Attackable_(info, tower, i);
		if (info.towerInfo[i].owner == info.myID
			|| Stemp <= 0)
			continue;
		else {
			Toweraims.push_back(i);
			S.push_back(Stemp);
		}
	}
	for (int i = 0; i < Toweraims.size(); i++)       //简单的资源距离由优到劣
		for (int j = i + 1; j < Toweraims.size(); j++) {
			if (S[i] < S[j]) {
				TTowerID temp = Toweraims[i];
				double tempS = S[i];
				Toweraims[i] = Toweraims[j];
				Toweraims[j] = temp;
				S[i] = S[j];
				S[j] = tempS;
			}
		}
}

//切断兵线
void Alchemist::CutLine_(Info& info) {
	for (int i = 0; i < Player[0].TowersID.size(); i++) {
		if (step >= info.myMaxControl)
			return;
		for (int j = 0; j < Tower[Player[0].TowersID[i]].Aimtowers.size(); j++) {
			if (step >= info.myMaxControl)
				return;
			//对我方塔
			if (info.towerInfo[Tower[Player[0].TowersID[i]].Aimtowers[j]].owner == info.myID) {
				//兵力最大塔距兵力上限的差小于10, 或者兵源被攻击
				if (info.towerInfo[Tower[Player[0].TowersID[i]].Aimtowers[j]].maxResource
					- info.towerInfo[Tower[Player[0].TowersID[i]].Aimtowers[j]].resource < 10
					|| Tower[Player[0].TowersID[i]].Attactktowers.size() > 0) {
					info.myCommandList.addCommand(cutLine, Player[0].TowersID[i],
						Tower[Player[0].TowersID[i]].Aimtowers[j],
						getDistance(info.towerInfo[Player[0].TowersID[i]].position,
							info.towerInfo[Tower[Player[0].TowersID[i]].Aimtowers[j]].position) / 10 - 1);      //切断对主塔的奶
					step++;
				}
				//塔内兵力小于40
				if (info.towerInfo[Player[0].TowersID[i]].resource < 40) {
					info.myCommandList.addCommand(cutLine, Player[0].TowersID[i],
						Tower[Player[0].TowersID[i]].Aimtowers[j],
						41 - info.towerInfo[Player[0].TowersID[i]].resource);      //切断对主塔的奶, 回复满40
					step++;
				}
			}
			//对敌方塔
			else {
				//如果对方反击
				if (info.lineInfo[Tower[Player[0].TowersID[i]].Aimtowers[j]][Player[0].TowersID[i]].exist) {
					if (info.towerInfo[Player[0].TowersID[i]].resource + 2 * info.lineInfo[Player[0].TowersID[i]][Tower[Player[0].TowersID[i]].Aimtowers[j]].resource
						< info.towerInfo[Tower[Player[0].TowersID[i]].Aimtowers[j]].resource + /*10 + */info.lineInfo[Player[0].TowersID[i]][Tower[Player[0].TowersID[i]].Aimtowers[j]].maxlength / 10) {
						info.myCommandList.addCommand(cutLine, Player[0].TowersID[i],
							Tower[Player[0].TowersID[i]].Aimtowers[j],
							getDistance(info.towerInfo[Player[0].TowersID[i]].position,
								info.towerInfo[Tower[Player[0].TowersID[i]].Aimtowers[j]].position) / 10 - 1);      //撤兵
						step++;
					}
				}
				//如果被攻击或者无法安全攻占对方塔
				else if (info.towerInfo[Player[0].TowersID[i]].resource + info.lineInfo[Player[0].TowersID[i]][Tower[Player[0].TowersID[i]].Aimtowers[j]].resource
					< info.towerInfo[Tower[Player[0].TowersID[i]].Aimtowers[j]].resource + 10
					|| Tower[Player[0].TowersID[i]].Attactktowers.size() > 1) {
					info.myCommandList.addCommand(cutLine, Player[0].TowersID[i],
						Tower[Player[0].TowersID[i]].Aimtowers[j],
						getDistance(info.towerInfo[Player[0].TowersID[i]].position,
							info.towerInfo[Tower[Player[0].TowersID[i]].Aimtowers[j]].position) / 10 - 1);      //撤兵
					step++;
				}
			}
		}



	}
}

//快速进攻
void Alchemist::QuiklyAttack_(Info& info) {
	for (int i = 0; i < Player[0].TowersID.size(); i++) {
		for (int j = 0; j < Tower[Player[0].TowersID[i]].Aimtowers.size(); j++) {
			if (step >= info.myMaxControl)
				return;

			bool QuiklyAttackAble = true;
			int M, E = 0;
			double MLs = info.lineInfo[Player[0].TowersID[i]][Tower[Player[0].TowersID[i]].Aimtowers[j]].resource;
			double ASs = Tower[Tower[Player[0].TowersID[i]].Aimtowers[j]].resourcesum;
			for (int k = 0; k < Tower[Tower[Player[0].TowersID[i]].Aimtowers[j]].Attactktowers.size(); k++) {
				if (info.towerInfo[Tower[Tower[Player[0].TowersID[i]].Aimtowers[j]].Attactktowers[k]].owner != info.myID)
					E++;
				else
					M++;
			}
			if (M > E + 1 && MLs > ASs + 3) {      //兵线上的兵力大于目标塔总兵力 + 3
			}
			else
				QuiklyAttackAble = false;

			if (QuiklyAttackAble) {
				info.myCommandList.addCommand(cutLine, Player[0].TowersID[i],
					Tower[Player[0].TowersID[i]].Aimtowers[j],
					MLs - ASs - 2);      //切断并快速进攻
				step++;
			}

		}
	}

}

//回复速率计算
double Alchemist::vGrow_(Info& info, TTowerID tower) {
	int n = info.towerInfo[tower].resource;
	double v0, vc, vs;      //基础、策略、势力

	if (n < 10)
		v0 = 1;
	else if (n < 40)
		v0 = 1.5;
	else if (n < 80)
		v0 = 2;
	else if (n < 150)
		v0 = 2.5;
	else
		v0 = 3;

	switch (Tower[tower].strategy)
	{
	case Defence:
		vc = 0.5;
		break;
	case Grow:
		vc = 1.5;
		break;
	default:
		vc = 1;
		break;
	}

	vs = 1 + 0.05*info.playerInfo[info.towerInfo[tower].owner].RegenerationSpeedLevel;

	return (v0*vc*vs);
}

//出兵速率
double Alchemist::v_(Info& info, TTowerID tower) {
	double v0 = 3;
	double vs;

	vs = 1 + 0.1*info.playerInfo[info.towerInfo[tower].owner].ExtendingSpeedLevel;

	return (v0*vs);
}

//消耗速率计算
double Alchemist::vDown_(Info& info, TTowerID tower) {
	int lineN = Tower[tower].Aimtowers.size();
	double v0 = v_(info, tower);
	double v1;

	v1 = lineN*(0.8 + 0.2*lineN);

	return (v0*v1);
}