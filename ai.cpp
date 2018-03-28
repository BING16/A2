#include "ai.h"
#include "definition.h"
#include "user_toolbox.h"
#include <iostream>
#include <vector>
#include <time.h>
#include <stdlib.h>

//数据类型定义
typedef int MYPID;
typedef int MYTID;
typedef struct MyPlayerInfo
{
	TPlayerID ID;
	MYPID MyID;      //在info.player中的序列, 用以直接查询info.player[MYID]中的信息
	double TResource = 0;      //势力的总资源数
	vector <MYTID> TowersMyID;
};

//数据定义
int step = 0;      //当前操作数，上限为info.myMaxControl
Command C;      //命令结构体
static vector<MyPlayerInfo> Player;      //玩家, 我方为[0]
static int alivenum = 0;      //剩余玩家数
int i, j, k;      //常用temp

//函数声明区
void Initialize_(Info& info);      //信息初始化
void HappyGrow_(Info& info);      //猥琐发育
bool Passable_(Info& info, MYTID tower1, MYTID tower2);      //判断能否正常出兵1.0

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

}


/*函数定义区*/

//初始化信息
void Initialize_(Info& info) {
	step = 0;

	//初始化Player[]信息
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

	for (i = 0; i < alivenum; i++)
		Player[i].TResource = 0;      //数据归零
	for (i = 0; i < info.towerNum; i++) {      //统计势力的总资源
		for (j = 0; j < 4; j++) {
			if (info.towerInfo[i].owner == Player[j].ID) {
				Player[j].TResource += info.towerInfo[i].resource;
				Player[j].TowersMyID.push_back(i);
				break;
			}
		}
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
	if(info.playerInfo[Player[0].MyID].technologyPoint>=
		RegenerationSpeedUpdateCost[info.playerInfo[Player[0].MyID].RegenerationSpeedLevel]
		&& info.playerInfo[Player[0].MyID].RegenerationSpeedLevel<= MAX_REGENERATION_SPEED_LEVEL) {
		info.myCommandList.addCommand(upgrade, RegenerationSpeed);
		step++;
	}

	//无脑传输
	for (i = 1; i < Player[0].TowersMyID.size(); i++) {
		if (step >= info.myMaxControl)
			return;
		else if ((info.towerInfo[Player[0].TowersMyID[i]].resource - 10) > 20
			&& info.towerInfo[Player[0].TowersMyID[i]].resource >= 10) {      //初始值大约在10
			if (Passable_(info, Player[0].TowersMyID[i], Player[0].TowersMyID[0])) {      //目标兵线不存在且兵力可到达
				info.myCommandList.addCommand(addLine, info.towerInfo[Player[0].TowersMyID[i]].id,
					info.towerInfo[Player[0].TowersMyID[0]].id);
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