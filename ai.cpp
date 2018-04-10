/**********************************--��־--**************************************
�汾��Ϣ:
v1.0
1. ��������ʵ��, �����Ż������㷨

v1.1
1. ���������ռ�, ����Ч����δ����

Ԥ�����Ӽ��Ż�������:
1.�Ż�Attackable_�ж��н��ٶ�v����������o����������o���ظ�����v��Ӱ��ļ���     o
2.����Reattack_�е�֧Ԯ�㷨                                                     o
3.�Ż�SearchBesAim_�ж���Ԯ�����ġ������ظ����������Ŀ���                       v
4.�Ż�Attack_�е�Χ����֧Ԯ�㷨                                                 o
5.�Ż�Passable_�жԻظ����ʼ�ʣ������Ŀ���                                     v
6.215��216����δ֪���ܵĴ���                              ��������, δ���ֳ�bug v
7.�Ƿ���Ҫ�����ϰ������                                         �ƺ�û���ϰ��� x
8.�Ż�CutLine_�н������ж����                                                  o
9.����ͼģʽ�вҰ�, �����ԭ��                    ����ԭ��δ֪, ����G���Ժ���ʤ v
 ---------------------------------
|../player_ai/ai/Release/ai.dll   |
|../sample_ai/sample_ai_ver1.3.dll|
|../sample_ai/random_ai_2.dll     |
|../sample_ai/sample_ai_ver1.1.dll|
 ---------------------------------

v1.2
1. ����һЩ����, ��ҳ�޷�ͨ�������������Ȼ����

Ԥ�����Ӽ��Ż�������:
1. �����ҳ���ҳ�޷�����ͨ����bug��ԭ��                     �ѽ��, Ϊ��վ������ v
2. ͬ���жϺ��������캯����һЩ�������趨                                       v
3. ����Χ������                                                                 o
4. �Ż�������Ŀ                                                                 o
5. ���̵����㷨��                                                               o
6. �Ż�CutLine_�н������ж����                                                 v
7. ���߱��������������, ����Ŭ��                                               o

v1.3
1. ����Attactktowers��ͳ�ƹ���
2. ���ӷ��������ȼ�
3. �������������������������������֮ǰ (�ڱ��ضԾ�����һ��Ч��) v1.3.5
4. ���ӿ��ٹ����㷨, ������ v1.3.6 v1.3.7 (�����߼�bug)
5. �������һ���㷨 (ͳ��������ӵ�е�������bug?) v1.3.5
6. ������������Դ��Ϣ
7. �ֶ���ռ12���� v1.3.8
8. �Ż������㷨���������� v1.3.9

v1.4
1. �޸�v1.3.7�з��ֵ�bug, ���Ż��㷨

*********************************************************************************/

#include "ai.h"
#include "definition.h"
#include "user_toolbox.h"
#include <math.h>
#include <iostream>
#include <vector>
#include <time.h>
#include <stdlib.h>

namespace Alchemist {      //����ʹ�������ռ����޷�������Ϸ������
	/*�������Ͷ���*/
	typedef struct MyPlayerInfo {
		TPlayerID ID;
		double TResource;      //����������Դ��
		vector <TTowerID> TowersID;      //��Ӧ��info.towerInfo��Tower�е�˳��
	};
	typedef struct MyTowerInfo {
		TTowerID ID;
		int currLineNum;      //��ǰ������ = info.towerInfo[MYID].currLineNum
		TResourceD resourcesum;      //����Դ
		TowerStrategy strategy;      //ʵʱ���µ�������
		vector <TTowerID> Aimtowers;      //����������
		vector <TTowerID> Attactktowers;      //������������
	};

	/*��������*/
	int step = 0;      //��ǰ������������Ϊinfo.myMaxControl
	Command C;      //����ṹ��
	vector<MyPlayerInfo> Player;      //���, �ҷ�Ϊ[0]
	vector<MyTowerInfo> Tower;      //��,˳���Ӧinfo.towerInfo[]��˳��
	vector <TTowerID> Toweraims;      //����������ز���

	/*����������*/
	void Initialize_(Info& info);      //��Ϣ��ʼ��
	void HappyGrow_(Info& info);      //�������
	bool Passable_(Info& info, TTowerID tower1, TTowerID tower2);      //�ж��ܷ���������1.0
	double Attackable_(Info& info, TTowerID tower1, TTowerID tower2);      //�ж��ܷ����1.0
	void Reattack_(Info& info, TTowerID tower1, TTowerID tower2);      //��������
	void Attack_(Info& info);      //��������
	void SearchBesAim_(Info& info, TTowerID tower);      //Ѱ�����Ž���Ŀ�� 
	void CutLine_(Info& info);      //�жϱ���
	void QuiklyAttack_(Info& info);      //�����ж�
	double vGrow_(Info& info, TTowerID tower);
	double vDown_(Info& info, TTowerID tower);
	double v_(Info& info, TTowerID tower);
}

using namespace Alchemist;

/*ѡ��������*/

void player_ai(Info& info)
{
	Initialize_(info);

	//�ȷ�ռ��12����
	if (info.round <= 20 && info.towerInfo[Player[0].TowersID[2]].resource > 20
		&& Tower[12].Attactktowers.size() == 0) {
		info.myCommandList.addCommand(addLine, Player[0].TowersID[2], 12);
		step++;
	}

	QuiklyAttack_(info);

	for (int i = 0; i < Player[0].TowersID.size(); i++) {      //��������
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
					&&Tower[LonelyTown].strategy != Defence) {      //����ģʽ
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


/*����������*/

//��ʼ����Ϣ
void Alchemist::Initialize_(Info& info) {
	step = 0;

	//����ʼ��Player[]��Ϣ�Լ�����ID�ı�ʱ��������
	Player.clear();
	MyPlayerInfo IDtemp;
	IDtemp.ID = info.myID;
	IDtemp.TResource = 0;
	Player.push_back(IDtemp);
	for (int i = 0; i < info.playerSize; i++) {      //��ʼ��ID����, kΪPlayer�����
		if (info.playerInfo[i].id != Player[0].ID&&info.playerInfo[i].alive) {      //���ҷ�����
			MyPlayerInfo IDtemp;
			IDtemp.ID = i;
			IDtemp.TResource = 0;
			Player.push_back(IDtemp);
		}
	}

	//ʵʱͳ��������Դ������Ϣ
	Tower.clear();      //���������
	for (int i = 0; i < info.towerNum; i++) {      //ͳ������������Դ
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
		for (int k = 0; k < info.towerNum; k++) {      //ͳ������Ϣ
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

	//������������Դ�ɴ�С����
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

//�������
void Alchemist::HappyGrow_(Info& info) {
	//����������������
	if (info.playerInfo[Player[0].ID].technologyPoint >=      //��Դ����
		RegenerationSpeedUpdateCost[info.playerInfo[Player[0].ID].RegenerationSpeedLevel]
		&& info.playerInfo[Player[0].ID].RegenerationSpeedLevel < MAX_REGENERATION_SPEED_LEVEL) {
		info.myCommandList.addCommand(upgrade, RegenerationSpeed);
		step++;
	}

	//�ı�������Ϊ����
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

	if (info.playerInfo[Player[0].ID].technologyPoint >=      //��������
		DefenceStageUpdateCost[info.playerInfo[Player[0].ID].DefenceLevel]
		&& info.playerInfo[Player[0].ID].DefenceLevel < MAX_DEFENCE_LEVEL) {
		info.myCommandList.addCommand(upgrade, Wall);
		step++;
	}
	if (info.playerInfo[Player[0].ID].technologyPoint >=      //�ж���
		DefenceStageUpdateCost[info.playerInfo[Player[0].ID].ExtraControlLevel]
		&& info.playerInfo[Player[0].ID].ExtraControlLevel < MAX_EXTRA_CONTROL_LEVEL) {
		info.myCommandList.addCommand(upgrade, ExtraControl);
		step++;
	}
	if (info.playerInfo[Player[0].ID].technologyPoint >=      //�ٶ�
		DefenceStageUpdateCost[info.playerInfo[Player[0].ID].ExtendingSpeedLevel]
		&& info.playerInfo[Player[0].ID].ExtendingSpeedLevel < MAX_EXTENDING_SPEED_LEVEL) {
		info.myCommandList.addCommand(upgrade, ExtendingSpeed);
		step++;
	}

	//���Դ���
	for (int i = 1; i < Player[0].TowersID.size(); i++) {
		if (step >= info.myMaxControl
			|| (Tower[i].Attactktowers.size() > 0      //�ܵ����������������������ޱ�����
			|| info.towerInfo[Player[0].TowersID[0]].maxResource - info.towerInfo[Player[0].TowersID[0]].resource < 15)
			)
			break;
		/*���ִ���ת����CutLine_()*/
		else if (Attackable_(info, Player[0].TowersID[i], Player[0].TowersID[0]) > 40
			&& info.towerInfo[Player[0].TowersID[i]].resource >= 10      //��ʼֵ��Լ��10
			&& info.towerInfo[Player[0].TowersID[0]].resource < info.towerInfo[Player[0].TowersID[0]].maxResource) {
			if (Passable_(info, Player[0].TowersID[i], Player[0].TowersID[0])) {      //Ŀ����߲������ұ����ɵ���
				info.myCommandList.addCommand(addLine, Player[0].TowersID[i], Player[0].TowersID[0]);
				step++;
			}
		}
	}
}

//�ж��ܷ���������2.0
bool Alchemist::Passable_(Info& info, TTowerID tower1, TTowerID tower2) {
	//��·�ж�
	if (tower1 == tower2      //ͬһ����
		|| info.mapInfo->passable(info.towerInfo[tower1].position,
			info.towerInfo[tower2].position) == false      //��·����
		|| info.lineInfo[tower1][tower2].exist) {      //���ߴ���
		return false;
	}

	double D = getDistance(info.towerInfo[tower1].position, info.towerInfo[tower2].position) / 10;     //�������

	//�����ж�
	if (info.towerInfo[tower1].currLineNum < info.towerInfo[tower1].maxLineNum      //���ɱ�
		&&info.towerInfo[tower1].resource >= D)      //�����㹻����
		return true;
	else
		return false;
}

//�ж��ܷ����2.0
double Alchemist::Attackable_(Info& info, TTowerID tower1, TTowerID tower2) {
	if (Passable_(info, tower1, tower2) == false)      //���ɳ���
		return 0;

	double D = getDistance(info.towerInfo[tower1].position, info.towerInfo[tower2].position) / 10;     //�������
	double t = D / v_(info, tower1);      //������Ҫ��ʱ��
	double S = info.towerInfo[tower1].resource + (vGrow_(info, tower1) * t - vDown_(info, tower1) * t);      //������Դ
	double A = Tower[tower2].resourcesum + vGrow_(info, tower2) * t;      //������Ŀ��

	//���㵽����Դ���ڵ�����Ŀ��+10
	if (S > D + A + 10)      //�Ƚ�ĳʱ�̵ı����������֮��ı���
		return S - D - A - 10;
	else
		return 0;
}

//��������������
void Alchemist::Reattack_(Info& info, TTowerID tower1, TTowerID tower2) {
	if (step < info.myMaxControl && Attackable_(info, tower1, tower2) == 0) {      //�޷��ɹ�����
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

//��������
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
					&&Tower[Toweraims[i]].strategy != Defence) {      //����ģʽ...����������ֱ������...
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

//Ѱ�����Ž���Ŀ�� 
void Alchemist::SearchBesAim_(Info& info, TTowerID tower) {
	Toweraims.clear();
	vector <double> S;
	double Stemp;
	for (int i = 0; i < info.towerNum; i++) {      //ͳ�ƿɽ���������Ϣ
		Stemp = Attackable_(info, tower, i);
		if (info.towerInfo[i].owner == info.myID
			|| Stemp <= 0)
			continue;
		else {
			Toweraims.push_back(i);
			S.push_back(Stemp);
		}
	}
	for (int i = 0; i < Toweraims.size(); i++)       //�򵥵���Դ�������ŵ���
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

//�жϱ���
void Alchemist::CutLine_(Info& info) {
	for (int i = 0; i < Player[0].TowersID.size(); i++) {
		if (step >= info.myMaxControl)
			return;
		for (int j = 0; j < Tower[Player[0].TowersID[i]].Aimtowers.size(); j++) {
			if (step >= info.myMaxControl)
				return;
			//���ҷ���
			if (info.towerInfo[Tower[Player[0].TowersID[i]].Aimtowers[j]].owner == info.myID) {
				//�����������������޵Ĳ�С��10, ���߱�Դ������
				if (info.towerInfo[Tower[Player[0].TowersID[i]].Aimtowers[j]].maxResource
					- info.towerInfo[Tower[Player[0].TowersID[i]].Aimtowers[j]].resource < 10
					|| Tower[Player[0].TowersID[i]].Attactktowers.size() > 0) {
					info.myCommandList.addCommand(cutLine, Player[0].TowersID[i],
						Tower[Player[0].TowersID[i]].Aimtowers[j],
						getDistance(info.towerInfo[Player[0].TowersID[i]].position,
							info.towerInfo[Tower[Player[0].TowersID[i]].Aimtowers[j]].position) / 10 - 1);      //�ж϶���������
					step++;
				}
				//���ڱ���С��40
				if (info.towerInfo[Player[0].TowersID[i]].resource < 40) {
					info.myCommandList.addCommand(cutLine, Player[0].TowersID[i],
						Tower[Player[0].TowersID[i]].Aimtowers[j],
						41 - info.towerInfo[Player[0].TowersID[i]].resource);      //�ж϶���������, �ظ���40
					step++;
				}
			}
			//�Եз���
			else {
				//����Է�����
				if (info.lineInfo[Tower[Player[0].TowersID[i]].Aimtowers[j]][Player[0].TowersID[i]].exist) {
					if (info.towerInfo[Player[0].TowersID[i]].resource + 2 * info.lineInfo[Player[0].TowersID[i]][Tower[Player[0].TowersID[i]].Aimtowers[j]].resource
						< info.towerInfo[Tower[Player[0].TowersID[i]].Aimtowers[j]].resource + /*10 + */info.lineInfo[Player[0].TowersID[i]][Tower[Player[0].TowersID[i]].Aimtowers[j]].maxlength / 10) {
						info.myCommandList.addCommand(cutLine, Player[0].TowersID[i],
							Tower[Player[0].TowersID[i]].Aimtowers[j],
							getDistance(info.towerInfo[Player[0].TowersID[i]].position,
								info.towerInfo[Tower[Player[0].TowersID[i]].Aimtowers[j]].position) / 10 - 1);      //����
						step++;
					}
				}
				//��������������޷���ȫ��ռ�Է���
				else if (info.towerInfo[Player[0].TowersID[i]].resource + info.lineInfo[Player[0].TowersID[i]][Tower[Player[0].TowersID[i]].Aimtowers[j]].resource
					< info.towerInfo[Tower[Player[0].TowersID[i]].Aimtowers[j]].resource + 10
					|| Tower[Player[0].TowersID[i]].Attactktowers.size() > 1) {
					info.myCommandList.addCommand(cutLine, Player[0].TowersID[i],
						Tower[Player[0].TowersID[i]].Aimtowers[j],
						getDistance(info.towerInfo[Player[0].TowersID[i]].position,
							info.towerInfo[Tower[Player[0].TowersID[i]].Aimtowers[j]].position) / 10 - 1);      //����
					step++;
				}
			}
		}



	}
}

//���ٽ���
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
			if (M > E + 1 && MLs > ASs + 3) {      //�����ϵı�������Ŀ�����ܱ��� + 3
			}
			else
				QuiklyAttackAble = false;

			if (QuiklyAttackAble) {
				info.myCommandList.addCommand(cutLine, Player[0].TowersID[i],
					Tower[Player[0].TowersID[i]].Aimtowers[j],
					MLs - ASs - 2);      //�жϲ����ٽ���
				step++;
			}

		}
	}

}

//�ظ����ʼ���
double Alchemist::vGrow_(Info& info, TTowerID tower) {
	int n = info.towerInfo[tower].resource;
	double v0, vc, vs;      //���������ԡ�����

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

//��������
double Alchemist::v_(Info& info, TTowerID tower) {
	double v0 = 3;
	double vs;

	vs = 1 + 0.1*info.playerInfo[info.towerInfo[tower].owner].ExtendingSpeedLevel;

	return (v0*vs);
}

//�������ʼ���
double Alchemist::vDown_(Info& info, TTowerID tower) {
	int lineN = Tower[tower].Aimtowers.size();
	double v0 = v_(info, tower);
	double v1;

	v1 = lineN*(0.8 + 0.2*lineN);

	return (v0*v1);
}