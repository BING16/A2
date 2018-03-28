#include "ai.h"
#include "definition.h"
#include "user_toolbox.h"
#include <iostream>
#include <vector>
#include <time.h>
#include <stdlib.h>

//�������Ͷ���
typedef int MYPID;
typedef int MYTID;
typedef struct MyPlayerInfo
{
	TPlayerID ID;
	MYPID MyID;      //��info.player�е�����, ����ֱ�Ӳ�ѯinfo.player[MYID]�е���Ϣ
	double TResource = 0;      //����������Դ��
	vector <MYTID> TowersMyID;
};

//���ݶ���
int step = 0;      //��ǰ������������Ϊinfo.myMaxControl
Command C;      //����ṹ��
static vector<MyPlayerInfo> Player;      //���, �ҷ�Ϊ[0]
static int alivenum = 0;      //ʣ�������
int i, j, k;      //����temp

//����������
void Initialize_(Info& info);      //��Ϣ��ʼ��
void HappyGrow_(Info& info);      //�������
bool Passable_(Info& info, MYTID tower1, MYTID tower2);      //�ж��ܷ���������1.0

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

}


/*����������*/

//��ʼ����Ϣ
void Initialize_(Info& info) {
	step = 0;

	//��ʼ��Player[]��Ϣ
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

	for (i = 0; i < alivenum; i++)
		Player[i].TResource = 0;      //���ݹ���
	for (i = 0; i < info.towerNum; i++) {      //ͳ������������Դ
		for (j = 0; j < 4; j++) {
			if (info.towerInfo[i].owner == Player[j].ID) {
				Player[j].TResource += info.towerInfo[i].resource;
				Player[j].TowersMyID.push_back(i);
				break;
			}
		}
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
	if(info.playerInfo[Player[0].MyID].technologyPoint>=
		RegenerationSpeedUpdateCost[info.playerInfo[Player[0].MyID].RegenerationSpeedLevel]
		&& info.playerInfo[Player[0].MyID].RegenerationSpeedLevel<= MAX_REGENERATION_SPEED_LEVEL) {
		info.myCommandList.addCommand(upgrade, RegenerationSpeed);
		step++;
	}

	//���Դ���
	for (i = 1; i < Player[0].TowersMyID.size(); i++) {
		if (step >= info.myMaxControl)
			return;
		else if ((info.towerInfo[Player[0].TowersMyID[i]].resource - 10) > 20
			&& info.towerInfo[Player[0].TowersMyID[i]].resource >= 10) {      //��ʼֵ��Լ��10
			if (Passable_(info, Player[0].TowersMyID[i], Player[0].TowersMyID[0])) {      //Ŀ����߲������ұ����ɵ���
				info.myCommandList.addCommand(addLine, info.towerInfo[Player[0].TowersMyID[i]].id,
					info.towerInfo[Player[0].TowersMyID[0]].id);
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