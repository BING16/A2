#include "ai.h"
#include "definition.h"
#include "user_toolbox.h"
#include <iostream>
#include <vector>
#include <time.h>
#include <stdlib.h>

/*数据类型定义*/
typedef int MYPID;
typedef int MYTID;
typedef struct MyPlayerInfo{
	TPlayerID ID;
	MYPID MyID;      //在info.playerInfo中的序列, 用以直接查询info.playerInfo[MYID]中的信息
	double TResource = 0;      //势力的总资源数
	vector <MYTID> TowersMyID;      //对应着info.towerInfo和Tower中的顺序
};
typedef struct MyTowerInfo {
	TTowerID ID;
	MYTID MyID;      //在info.towerInfo中的序列, 用以直接查询info.towerInfo[MYID]中的信息
	int currLineNum;      //当前兵线数 = info.towerInfo[MYID].currLineNum
	TowerStrategy strategy;      //实时更新的塔属性
	vector <MYTID> Aimtowers;      //正进攻的塔
	vector <MYTID> Attactktowers;      //进攻该塔的塔
};

/*数据定义*/
int step = 0;      //当前操作数，上限为info.myMaxControl
Command C;      //命令结构体
static vector<MyPlayerInfo> Player;      //玩家, 我方为[0]
static vector<MyTowerInfo> Tower;      //塔,顺序对应info.towerInfo[]中顺序
static int alivenum = 0;      //剩余玩家数
int i, j, k;      //常用temp

/*函数声明区*/
void Initialize_(Info& info);      //信息初始化
void HappyGrow_(Info& info);      //猥琐发育
bool Passable_(Info& info, MYTID tower1, MYTID tower2);      //判断能否正常出兵1.0
bool Attackable_(Info& info, MYTID tower1, MYTID tower2);      //判断能否进攻1.0
void Reattack_(Info& info, MYTID tower1, MYTID tower2);      //反击函数

/*选手主函数*/
void player_ai(Info& info)
{
	Initialize_(info);
	
	int Dmax = 0;      //其他势力总资源大于我方总资源的量的最大值
	for (i = 1; i < 4; i++)
		if (Dmax <= (Player[i].TResource - Player[0].TResource))
			(Dmax = (Player[i].TResource - Player[0].TResource));
	//if (Player[0].TResource < 100&&Dmax<=30) {      //触发猥琐发育
	HappyGrow_(info);
	//}

	for (i = 0; i < Player[0].TowersMyID.size(); i++) {      //触发反击
		if (step >= info.myMaxControl)
			break;
		if (Tower[Player[0].TowersMyID[i]].Attactktowers.size() > 0) {
			for (j = 0; j < Tower[Player[0].TowersMyID[i]].Attactktowers.size(); j++)
				Reattack_(info, Player[0].TowersMyID[i], Tower[Player[0].TowersMyID[i]].Attactktowers[j]);
		}
	}
}


/*函数定义区*/

//初始化信息
void Initialize_(Info& info) {
	step = 0;

	//初初始化Player[]信息以及势力ID改变时更新数据
	if (alivenum != info.playerAlive) {
		alivenum = info.playerAlive;      //剩余玩家数赋值
		if (Player.size() != 0)
			Player[0].ID = info.myID;
		else {
			MyPlayerInfo IDtemp;
			IDtemp.ID = info.myID;
			Player.push_back(IDtemp);
		}
		for (i = 0, k = 1; i < alivenum; i++) {      //初始化ID数组
			if (info.playerInfo[i].id != Player[0].ID) {      //k为Player的序号
				if (Player.size() >= alivenum) {
					Player[k].MyID = i;
					Player[k].ID = info.playerInfo[i].id;
					k++;
				}
				else {
					MyPlayerInfo IDtemp;
					IDtemp.ID = i;
					IDtemp.ID = info.playerInfo[i].id;
					Player.push_back(IDtemp);      //第k个
					k++;
				}
			}
			else 
				Player[0].MyID = i;
		}
	}

	//实时统计势力资源及塔信息
	for (i = 0; i < alivenum; i++) {
		Player[i].TResource = 0;      //势力总资源数据归零
		Player[i].TowersMyID.clear();      //清空势力的塔信息
		Tower.clear();      //清空塔数组
	}
	for (i = 0; i < info.towerNum; i++) {      //统计势力的总资源
		for (j = 0; j < 4; j++) {
			if (info.towerInfo[i].owner == Player[j].ID) {
				Player[j].TResource += info.towerInfo[i].resource;
				Player[j].TowersMyID.push_back(i);      //对应着info.towerInfo和Tower中的顺序
				break;
			}
		}
		MyTowerInfo temptower;
		temptower.ID = i;
		temptower.MyID = i;
		temptower.strategy = info.towerInfo[i].strategy;
		for (k = 0; k < info.towerNum; k++) {      //统计塔信息
			if (info.lineInfo[i][k].exist)
				temptower.Aimtowers.push_back(k);
			else if (info.lineInfo[k][i].exist)
				temptower.Attactktowers.push_back(k);
		}
		Tower.push_back(temptower);
	}

	//各方塔按资源由大到小排序
	for (k = 0; k < alivenum; k++)
		for (i = 1; i < Player[k].TowersMyID.size(); i++)
			for (j = i + 1; j < Player[k].TowersMyID.size(); j++)
				if (info.towerInfo[Player[k].TowersMyID[i]].resource < info.towerInfo[Player[k].TowersMyID[j]].resource) {
					//Player[0].TowersMyID.swap(i, j);
					int temp = Player[k].TowersMyID[i];
					temp = Player[k].TowersMyID[i] = temp = Player[k].TowersMyID[j];
					temp = Player[k].TowersMyID[j] = temp;
				}

}

//猥琐发育
void HappyGrow_(Info& info) {
	//无脑势力属性提升
	if(info.playerInfo[Player[0].MyID].technologyPoint>=      //资源再生
		RegenerationSpeedUpdateCost[info.playerInfo[Player[0].MyID].RegenerationSpeedLevel]
		&& info.playerInfo[Player[0].MyID].RegenerationSpeedLevel< MAX_REGENERATION_SPEED_LEVEL) {
		info.myCommandList.addCommand(upgrade, RegenerationSpeed);
		step++;
	}
	if (info.playerInfo[Player[0].MyID].technologyPoint >=      //防御能力
		DefenceStageUpdateCost[info.playerInfo[Player[0].MyID].DefenceLevel]
		&& info.playerInfo[Player[0].MyID].DefenceLevel< MAX_DEFENCE_LEVEL) {
		info.myCommandList.addCommand(upgrade, Wall);
		step++;
	}
	if (info.playerInfo[Player[0].MyID].technologyPoint >=      //行动力
		DefenceStageUpdateCost[info.playerInfo[Player[0].MyID].ExtraControlLevel]
		&& info.playerInfo[Player[0].MyID].ExtraControlLevel< MAX_EXTRA_CONTROL_LEVEL) {
		info.myCommandList.addCommand(upgrade, ExtraControl);
		step++;
	}
	if (info.playerInfo[Player[0].MyID].technologyPoint >=      //速度
		DefenceStageUpdateCost[info.playerInfo[Player[0].MyID].ExtendingSpeedLevel]
		&& info.playerInfo[Player[0].MyID].ExtendingSpeedLevel< MAX_EXTENDING_SPEED_LEVEL) {
		info.myCommandList.addCommand(upgrade, ExtendingSpeed);
		step++;
	}

	//无脑传输
	for (i = 1; i < Player[0].TowersMyID.size(); i++) {
		if (step >= info.myMaxControl)
			return;
		else if ((info.towerInfo[Player[0].TowersMyID[i]].resource - 10) > 20
			&& info.towerInfo[Player[0].TowersMyID[i]].resource >= 10      //初始值大约在10
			&& info.towerInfo[Player[0].TowersMyID[0]].resource < info.towerInfo[Player[0].TowersMyID[0]].maxResource) {
			if (Passable_(info, Player[0].TowersMyID[i], Player[0].TowersMyID[0])) {      //目标兵线不存在且兵力可到达
				info.myCommandList.addCommand(addLine, Player[0].TowersMyID[i], Player[0].TowersMyID[0]);
				step++;
			}
		}
		else if (info.towerInfo[Player[0].TowersMyID[0]].maxResource - info.towerInfo[Player[0].TowersMyID[0]].resource < 10) {
			if (info.lineInfo[Player[0].TowersMyID[i]][Player[0].TowersMyID[0]].exist) {      //原兵线仍存在
				info.myCommandList.addCommand(cutLine, Player[0].TowersMyID[i], Player[0].TowersMyID[0]);
				step++;
			}
			if (Passable_(info, Player[0].TowersMyID[i], Player[0].TowersMyID[0])      //目标兵线不存在且兵力可到达
				&& step < info.myMaxControl) {
				info.myCommandList.addCommand(addLine, Player[0].TowersMyID[0], Player[0].TowersMyID[i]);
				step++;
			}
		}
	}
}


//判断能否正常出兵1.0 不考虑速率问题
bool Passable_(Info& info, MYTID tower1, MYTID tower2) {
	//线路判断
	if (tower1==tower2      //同一个塔
		||info.mapInfo->passable(info.towerInfo[tower1].position,
		info.towerInfo[tower2].position) == 0      //无路可走
		|| info.lineInfo[tower1][tower2].exist)      //兵线存在
		return false;

	double D = getDistance(info.towerInfo[tower1].position, info.towerInfo[tower2].position);     //计算距离

	//兵力判断
	if (info.towerInfo[tower1].currLineNum < info.towerInfo[tower1].maxLineNum      //可派兵
		&&info.towerInfo[tower1].resource >= D)      //兵力足够出征
		return true;
	else
		return false;
}

//判断能否进攻1.0
bool Attackable_(Info& info, MYTID tower1, MYTID tower2) {
	if (Passable_(info, tower1, tower2) == false)      //可出征
		return false;

	double D = getDistance(info.towerInfo[tower1].position, info.towerInfo[tower2].position);     //计算距离
	if (info.towerInfo[tower1].resource >= D + info.towerInfo[tower2].resource)      //比较某时刻的兵力，需计算之后的兵力
		return true;
	else
		return false;
}

//反击函数
void Reattack_(Info& info, MYTID tower1, MYTID tower2) {
	if(Attackable_(info, tower1, tower2)==false)
		if (Tower[tower1].strategy != Defence
			&&info.towerInfo[tower2].strategy == Attack
			&&info.playerInfo[Player[0].MyID].technologyPoint>=3) {
			info.myCommandList.addCommand(changeStrategy, tower1, Defence);
			Tower[tower1].strategy = Defence;
			step++;
		}
	if (step < info.playerInfo[Player[0].MyID].maxControlNumber) {
		info.myCommandList.addCommand(addLine, tower1, tower2);
		step++;
	}
}