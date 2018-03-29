#include "ai.h"
#include "definition.h"
#include "user_toolbox.h"
#include <iostream>
#include <vector>
#include <time.h>
#include <stdlib.h>

/*�������Ͷ���*/
typedef int MYPID;
typedef int MYTID;
typedef struct MyPlayerInfo{
	TPlayerID ID;
	MYPID MyID;      //��info.playerInfo�е�����, ����ֱ�Ӳ�ѯinfo.playerInfo[MYID]�е���Ϣ
	double TResource = 0;      //����������Դ��
	vector <MYTID> TowersMyID;      //��Ӧ��info.towerInfo��Tower�е�˳��
};
typedef struct MyTowerInfo {
	TTowerID ID;
	MYTID MyID;      //��info.towerInfo�е�����, ����ֱ�Ӳ�ѯinfo.towerInfo[MYID]�е���Ϣ
	int currLineNum;      //��ǰ������ = info.towerInfo[MYID].currLineNum
	TowerStrategy strategy;      //ʵʱ���µ�������
	vector <MYTID> Aimtowers;      //����������
	vector <MYTID> Attactktowers;      //������������
};

/*���ݶ���*/
int step = 0;      //��ǰ������������Ϊinfo.myMaxControl
Command C;      //����ṹ��
static vector<MyPlayerInfo> Player;      //���, �ҷ�Ϊ[0]
static vector<MyTowerInfo> Tower;      //��,˳���Ӧinfo.towerInfo[]��˳��
static int alivenum = 0;      //ʣ�������
int i, j, k;      //����temp

/*����������*/
void Initialize_(Info& info);      //��Ϣ��ʼ��
void HappyGrow_(Info& info);      //�������
bool Passable_(Info& info, MYTID tower1, MYTID tower2);      //�ж��ܷ���������1.0
bool Attackable_(Info& info, MYTID tower1, MYTID tower2);      //�ж��ܷ����1.0
void Reattack_(Info& info, MYTID tower1, MYTID tower2);      //��������

/*ѡ��������*/
void player_ai(Info& info)
{
	Initialize_(info);
	
	int Dmax = 0;      //������������Դ�����ҷ�����Դ���������ֵ
	for (i = 1; i < 4; i++)
		if (Dmax <= (Player[i].TResource - Player[0].TResource))
			(Dmax = (Player[i].TResource - Player[0].TResource));
	//if (Player[0].TResource < 100&&Dmax<=30) {      //�����������
	HappyGrow_(info);
	//}

	for (i = 0; i < Player[0].TowersMyID.size(); i++) {      //��������
		if (step >= info.myMaxControl)
			break;
		if (Tower[Player[0].TowersMyID[i]].Attactktowers.size() > 0) {
			for (j = 0; j < Tower[Player[0].TowersMyID[i]].Attactktowers.size(); j++)
				Reattack_(info, Player[0].TowersMyID[i], Tower[Player[0].TowersMyID[i]].Attactktowers[j]);
		}
	}
}


/*����������*/

//��ʼ����Ϣ
void Initialize_(Info& info) {
	step = 0;

	//����ʼ��Player[]��Ϣ�Լ�����ID�ı�ʱ��������
	if (alivenum != info.playerAlive) {
		alivenum = info.playerAlive;      //ʣ���������ֵ
		if (Player.size() != 0)
			Player[0].ID = info.myID;
		else {
			MyPlayerInfo IDtemp;
			IDtemp.ID = info.myID;
			Player.push_back(IDtemp);
		}
		for (i = 0, k = 1; i < alivenum; i++) {      //��ʼ��ID����
			if (info.playerInfo[i].id != Player[0].ID) {      //kΪPlayer�����
				if (Player.size() >= alivenum) {
					Player[k].MyID = i;
					Player[k].ID = info.playerInfo[i].id;
					k++;
				}
				else {
					MyPlayerInfo IDtemp;
					IDtemp.ID = i;
					IDtemp.ID = info.playerInfo[i].id;
					Player.push_back(IDtemp);      //��k��
					k++;
				}
			}
			else 
				Player[0].MyID = i;
		}
	}

	//ʵʱͳ��������Դ������Ϣ
	for (i = 0; i < alivenum; i++) {
		Player[i].TResource = 0;      //��������Դ���ݹ���
		Player[i].TowersMyID.clear();      //�������������Ϣ
		Tower.clear();      //���������
	}
	for (i = 0; i < info.towerNum; i++) {      //ͳ������������Դ
		for (j = 0; j < 4; j++) {
			if (info.towerInfo[i].owner == Player[j].ID) {
				Player[j].TResource += info.towerInfo[i].resource;
				Player[j].TowersMyID.push_back(i);      //��Ӧ��info.towerInfo��Tower�е�˳��
				break;
			}
		}
		MyTowerInfo temptower;
		temptower.ID = i;
		temptower.MyID = i;
		temptower.strategy = info.towerInfo[i].strategy;
		for (k = 0; k < info.towerNum; k++) {      //ͳ������Ϣ
			if (info.lineInfo[i][k].exist)
				temptower.Aimtowers.push_back(k);
			else if (info.lineInfo[k][i].exist)
				temptower.Attactktowers.push_back(k);
		}
		Tower.push_back(temptower);
	}

	//����������Դ�ɴ�С����
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

//�������
void HappyGrow_(Info& info) {
	//����������������
	if(info.playerInfo[Player[0].MyID].technologyPoint>=      //��Դ����
		RegenerationSpeedUpdateCost[info.playerInfo[Player[0].MyID].RegenerationSpeedLevel]
		&& info.playerInfo[Player[0].MyID].RegenerationSpeedLevel< MAX_REGENERATION_SPEED_LEVEL) {
		info.myCommandList.addCommand(upgrade, RegenerationSpeed);
		step++;
	}
	if (info.playerInfo[Player[0].MyID].technologyPoint >=      //��������
		DefenceStageUpdateCost[info.playerInfo[Player[0].MyID].DefenceLevel]
		&& info.playerInfo[Player[0].MyID].DefenceLevel< MAX_DEFENCE_LEVEL) {
		info.myCommandList.addCommand(upgrade, Wall);
		step++;
	}
	if (info.playerInfo[Player[0].MyID].technologyPoint >=      //�ж���
		DefenceStageUpdateCost[info.playerInfo[Player[0].MyID].ExtraControlLevel]
		&& info.playerInfo[Player[0].MyID].ExtraControlLevel< MAX_EXTRA_CONTROL_LEVEL) {
		info.myCommandList.addCommand(upgrade, ExtraControl);
		step++;
	}
	if (info.playerInfo[Player[0].MyID].technologyPoint >=      //�ٶ�
		DefenceStageUpdateCost[info.playerInfo[Player[0].MyID].ExtendingSpeedLevel]
		&& info.playerInfo[Player[0].MyID].ExtendingSpeedLevel< MAX_EXTENDING_SPEED_LEVEL) {
		info.myCommandList.addCommand(upgrade, ExtendingSpeed);
		step++;
	}

	//���Դ���
	for (i = 1; i < Player[0].TowersMyID.size(); i++) {
		if (step >= info.myMaxControl)
			return;
		else if ((info.towerInfo[Player[0].TowersMyID[i]].resource - 10) > 20
			&& info.towerInfo[Player[0].TowersMyID[i]].resource >= 10      //��ʼֵ��Լ��10
			&& info.towerInfo[Player[0].TowersMyID[0]].resource < info.towerInfo[Player[0].TowersMyID[0]].maxResource) {
			if (Passable_(info, Player[0].TowersMyID[i], Player[0].TowersMyID[0])) {      //Ŀ����߲������ұ����ɵ���
				info.myCommandList.addCommand(addLine, Player[0].TowersMyID[i], Player[0].TowersMyID[0]);
				step++;
			}
		}
		else if (info.towerInfo[Player[0].TowersMyID[0]].maxResource - info.towerInfo[Player[0].TowersMyID[0]].resource < 10) {
			if (info.lineInfo[Player[0].TowersMyID[i]][Player[0].TowersMyID[0]].exist) {      //ԭ�����Դ���
				info.myCommandList.addCommand(cutLine, Player[0].TowersMyID[i], Player[0].TowersMyID[0]);
				step++;
			}
			if (Passable_(info, Player[0].TowersMyID[i], Player[0].TowersMyID[0])      //Ŀ����߲������ұ����ɵ���
				&& step < info.myMaxControl) {
				info.myCommandList.addCommand(addLine, Player[0].TowersMyID[0], Player[0].TowersMyID[i]);
				step++;
			}
		}
	}
}


//�ж��ܷ���������1.0 ��������������
bool Passable_(Info& info, MYTID tower1, MYTID tower2) {
	//��·�ж�
	if (tower1==tower2      //ͬһ����
		||info.mapInfo->passable(info.towerInfo[tower1].position,
		info.towerInfo[tower2].position) == 0      //��·����
		|| info.lineInfo[tower1][tower2].exist)      //���ߴ���
		return false;

	double D = getDistance(info.towerInfo[tower1].position, info.towerInfo[tower2].position);     //�������

	//�����ж�
	if (info.towerInfo[tower1].currLineNum < info.towerInfo[tower1].maxLineNum      //���ɱ�
		&&info.towerInfo[tower1].resource >= D)      //�����㹻����
		return true;
	else
		return false;
}

//�ж��ܷ����1.0
bool Attackable_(Info& info, MYTID tower1, MYTID tower2) {
	if (Passable_(info, tower1, tower2) == false)      //�ɳ���
		return false;

	double D = getDistance(info.towerInfo[tower1].position, info.towerInfo[tower2].position);     //�������
	if (info.towerInfo[tower1].resource >= D + info.towerInfo[tower2].resource)      //�Ƚ�ĳʱ�̵ı����������֮��ı���
		return true;
	else
		return false;
}

//��������
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