
// RobotControllerGUIDlg.cpp : implementation file

#include "stdafx.h"
#include "RobotControllerGUI.h"
#include "RobotControllerGUIDlg.h"
#include "afxdialogex.h"
#include "PktDef.h"
#include "MySocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CRobotControllerGUIDlg dialog
extern bool ExeComplete = false;

void cmdThread(CRobotControllerGUIDlg *obj)
{
	MySocket comSocket(SocketType::CLIENT, obj->getIPAddress(), obj->getComPort(), ConnectionType::TCP, 100);
	comSocket.ConnectTCP();

	CString connectedMsg(_T("Connected!"));
	TCHAR connected[11];
	lstrcpy(connected, connectedMsg);
	obj->populateSystemMessages(connected);

	bool isCommand = false;
	unsigned short int dir = 0, dur = 0;
	char rxBuffer[128];
	char* txBuffer;
	char* ptr = nullptr;
	std::stringstream RawData;

	PktDef txPkt;
	MotorBody bodyData;
	CmdType userCmd, cmd;

	while (ExeComplete == false)
	{
		if (obj->getButtonSend()) {
			if (obj->getDrive())
				userCmd = DRIVE;
			else if (obj->getArm())
				userCmd = ARM;
			else if (obj->getClaw())
				userCmd = CLAW;
			else if (obj->getSleep())
				userCmd = SLEEP;

			txPkt.SetCmd(userCmd);
			cmd = txPkt.GetCmd();

			if (cmd != SLEEP) {
				if (cmd == DRIVE) {
					dir = obj->getSelectedSubCommand();

					if (dir == 0)
						dir = 2;
					else if (dir == 1)
						dir = 1;
					else if (dir == 2)
						dir = 4;
					else if (dir == 3)
						dir = 3;

					dur = obj->getSelectedParameter() + 1;
				}
				else if (cmd == ARM) {
					dir = obj->getSelectedSubCommand();
					dur = 0;

					if (dir == 0)
						dir = 6;
					else if(dir == 1)
						dir = 5;
				}
				else if (cmd == CLAW) {
					dir = obj->getSelectedSubCommand();
					dur = 0;

					if (dir == 0)
						dir = 8;
					else if (dir == 1)
						dir = 7;
				}
				bodyData.direction = (char)dir;
				bodyData.duration = (char)dur;
				txPkt.SetBodyData((char*)&bodyData, sizeof(bodyData));

				txBuffer = new char[txPkt.GetLength()];
				txBuffer = txPkt.GenPacket();
				ptr = txBuffer;

				RawData.clear();
				RawData.str("");
				for (int i = 0; i < (int)txPkt.GetLength(); i++) 
					RawData << std::hex << std::setw(4) << (unsigned int)*(ptr++) << " ";

				std::cout << std::dec << std::endl;

				std::string str = RawData.str();

				CString rawMsg(str.c_str());
				TCHAR raw[100];
				lstrcpy(raw, rawMsg);
				CString txMsg(_T("Tx"));
				TCHAR tx[3];
				lstrcpy(tx, txMsg);
				obj->populateRAWData(raw, tx);
			}
			else if (cmd == SLEEP)
				txPkt.SetBodyData((char*)&bodyData, 0);

			txPkt.SetPktCount(1); //increment pktCount by 1
			txPkt.CalcCRC();

			txBuffer = new char[txPkt.GetLength()];
			txBuffer = txPkt.GenPacket();

			comSocket.SendData(txBuffer, txPkt.GetLength());
			comSocket.GetData(rxBuffer);

			PktDef rxPkt(rxBuffer);
			ptr = rxBuffer;

			RawData.clear();
			RawData.str("");
			for (int i = 0; i < (int)rxPkt.GetLength(); i++)
				RawData << std::hex << std::setw(4) << (unsigned int)*(ptr++) << " ";

			std::cout << std::dec << std::endl;

			std::string str = RawData.str();

			CString rawMsg(str.c_str());
			TCHAR raw[100];
			lstrcpy(raw, rawMsg);
			CString rxMsg(_T("Rx"));
			TCHAR rx[3];
			lstrcpy(rx, rxMsg);
			obj->populateRAWData(raw, rx);

			if ((txPkt.GetCmd() == SLEEP) && (rxPkt.GetCmd() == SLEEP) && rxPkt.GetAck())
			{
				comSocket.DisconnectTCP();
				ExeComplete = true;

				CString disconnectedMsg(_T("Disconnected!"));
				TCHAR disconnected[14];
				lstrcpy(disconnected, disconnectedMsg);
				obj->populateSystemMessages(disconnected);
			}
			else if (rxPkt.GetAck() == true) {
				CString commandMsg(_T("Command Accepted!"));
				TCHAR command[18];
				lstrcpy(command, commandMsg);
				obj->populateSystemMessages(command);
			}
			else if (rxPkt.GetCmd() == NACK && rxPkt.GetAck() == false) {
				CString rejectedMsg(_T("Rejected. Try again."));
				TCHAR rejected[20];
				lstrcpy(rejected, rejectedMsg);
				obj->populateSystemMessages(rejected);
			}

			obj->setButtonSendFalse();
		}
	}
}

void telThread(CRobotControllerGUIDlg *obj)
{
	MySocket telSocket(SocketType::CLIENT, obj->getIPAddress(), obj->getTelPort(), ConnectionType::TCP, 100);
	telSocket.ConnectTCP();

	unsigned short int vals[2];

	while (1)
	{
		char rxBuffer[100];
		char* ptr = rxBuffer;
		TelBody data;
		telSocket.GetData(rxBuffer);
		std::stringstream RawData;

		PktDef rxPkt(rxBuffer);
		bool result = rxPkt.CheckCRC(rxBuffer, rxPkt.GetLength());

		if (result)
		{
			if (rxPkt.GetStatus())
			{
				for (int i = 0; i < (int)rxPkt.GetLength(); i++)
					RawData << std::hex << std::setw(2) << (unsigned int)*(ptr++) << " ";

				std::cout << std::dec << std::endl;

				std::string str = RawData.str();

				CString rawMsg(str.c_str());
				TCHAR raw[100];
				lstrcpy(raw, rawMsg);
				CString telMsg(_T("Tel"));
				TCHAR tel[4];
				lstrcpy(tel, telMsg);
				obj->populateRAWData(raw, tel);

				char pktCount = rxPkt.GetPktCount();

				unsigned short int* uiptr = (unsigned short int*)rxPkt.GetBodyData();

				for (int i = 0; i < 2; i++)
					vals[i] = *uiptr++;

				std::string s = std::to_string(vals[0]);
				CString sonarVal(s.c_str());
				TCHAR sonarV[2];
				lstrcpy(sonarV, sonarVal);

				s = std::to_string(vals[1]);
				CString armVal(s.c_str());
				TCHAR armV[2];
				lstrcpy(armV, armVal);

				std::cout << std::endl;
				ptr = rxPkt.GetBodyData() + (sizeof(unsigned short int) * 2);
				data.Drive = *ptr & 1;
				data.ArmUp = (*ptr >> 1) & 1;
				data.ArmDown = (*ptr >> 2) & 1;
				data.ClawOpen = (*ptr >> 3) & 1;
				data.ClawClosed = (*ptr >> 4) & 1;
				data.Padding = 0;

				TCHAR d[5];
				_stprintf(d, TEXT("%d"), (bool)data.Drive);

				CString pktCount1;
				TCHAR count[4];
				_stprintf(count, TEXT("%d"), rxPkt.GetPktCount());

				if (data.ArmUp == 1 && data.ClawOpen == 1) {
					CString armMsg(_T("Up"));
					TCHAR arm[3];
					lstrcpy(arm, armMsg);

					CString clawMsg(_T("Open"));
					TCHAR claw[3];
					lstrcpy(claw, clawMsg);

					obj->populatePacketInfo(count, d, sonarV, armV, arm, claw);
				}
				else if (data.ArmUp == 1 && data.ClawClosed == 1) {
					CString armMsg(_T("Up"));
					TCHAR arm[3];
					lstrcpy(arm, armMsg);

					CString clawMsg(_T("Closed"));
					TCHAR claw[7];
					lstrcpy(claw, clawMsg);

					obj->populatePacketInfo(count, d, sonarV, armV, arm, claw);
				}
				else if (data.ArmDown == 1 && data.ClawOpen == 1) {
					CString armMsg(_T("Down"));
					TCHAR arm[5];
					lstrcpy(arm, armMsg);

					CString clawMsg(_T("Open"));
					TCHAR claw[3];
					lstrcpy(claw, clawMsg);

					obj->populatePacketInfo(count, d, sonarV, armV, arm, claw);
				}
				else if (data.ArmDown == 1 && data.ClawClosed == 1) {
					CString armMsg(_T("Down"));
					TCHAR arm[5];
					lstrcpy(arm, armMsg);

					CString clawMsg(_T("Closed"));
					TCHAR claw[7];
					lstrcpy(claw, clawMsg);

					obj->populatePacketInfo(count, d, sonarV, armV, arm, claw);
				}
			}
			else
			{
				CString invalidMsg(_T("Invalid Command."));
				TCHAR invalid[17];
				lstrcpy(invalid, invalidMsg);
				obj->populateSystemMessages(invalid);
			}
		}
		else
		{
			CString invalidMsg(_T("Invalid CRC."));
			TCHAR invalid[13];
			lstrcpy(invalid, invalidMsg);
			obj->populateSystemMessages(invalid);
		}
	}
}

CRobotControllerGUIDlg::CRobotControllerGUIDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_ROBOTCONTROLLERGUI_DIALOG, pParent)
	, Drive(false)
	, Arm(false)
	, Claw(false)
	, Sleep(false)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRobotControllerGUIDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPADDRESS, IPAddress);
	DDX_Control(pDX, IDC_COMMANDPORT, comPort);
	DDX_Control(pDX, IDC_TELEMETRYPORT, telPort);
	DDX_Control(pDX, IDC_SUBCOMMAND, subCommandList);
	DDX_Control(pDX, IDC_PARAMS, parametersList);
	DDX_Control(pDX, IDC_LIST1, PacketInfo);
	DDX_Control(pDX, IDC_LIST3, RAWData);
	DDX_Control(pDX, IDC_LIST4, SystemMessages);
}

BEGIN_MESSAGE_MAP(CRobotControllerGUIDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_DRIVE, &CRobotControllerGUIDlg::OnBnClickedDrive)
	ON_BN_CLICKED(IDC_ARM, &CRobotControllerGUIDlg::OnBnClickedArm)
	ON_BN_CLICKED(IDC_CLAW, &CRobotControllerGUIDlg::OnBnClickedClaw)
	ON_BN_CLICKED(IDC_SLEEP, &CRobotControllerGUIDlg::OnBnClickedSleep)
	ON_BN_CLICKED(IDC_CONNECTBUTTON, &CRobotControllerGUIDlg::OnBnClickedConnectbutton)
	ON_BN_CLICKED(IDC_SENDBUTTON, &CRobotControllerGUIDlg::OnBnClickedSendbutton)
	ON_NOTIFY(IPN_FIELDCHANGED, IDC_IPADDRESS, &CRobotControllerGUIDlg::OnIpnFieldchangedIpaddress)
END_MESSAGE_MAP()


// CRobotControllerGUIDlg message handlers

BOOL CRobotControllerGUIDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	IPAddress.SetAddress(127,0,0,1);
	comPort.SetWindowTextW(_T("27000"));
	telPort.SetWindowTextW(_T("27501"));

	PacketInfo.InsertColumn(0, _T("PktCount"), LVCFMT_LEFT, 120);
	PacketInfo.InsertColumn(1, _T("Drive Flag"), LVCFMT_LEFT, 140);
	PacketInfo.InsertColumn(2, _T("Sonar Value"), LVCFMT_LEFT, 165);
	PacketInfo.InsertColumn(3, _T("Arm Value"), LVCFMT_LEFT, 140);
	PacketInfo.InsertColumn(4, _T("Arm Status"), LVCFMT_LEFT, 165);
	PacketInfo.InsertColumn(5, _T("Claw Status"), LVCFMT_LEFT, 150);

	RAWData.InsertColumn(0, _T("RAW Data"), LVCFMT_LEFT, 650);
	RAWData.InsertColumn(1, _T("Tx/Rx/Tel"), LVCFMT_LEFT, 150);

	SystemMessages.InsertColumn(0, _T("System Messages"), LVCFMT_LEFT, 1500);

	setButtonConnectFalse();
	setButtonSendFalse();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRobotControllerGUIDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRobotControllerGUIDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRobotControllerGUIDlg::OnBnClickedDrive()
{
	// Clear Lists
	subCommandList.ResetContent();
	parametersList.ResetContent();

	// Add New Items
	subCommandList.AddString(_T("FORWARD"));
	subCommandList.AddString(_T("BACKWARD"));
	subCommandList.AddString(_T("LEFT"));
	subCommandList.AddString(_T("RIGHT"));
	parametersList.AddString(_T("1"));
	parametersList.AddString(_T("2"));
	parametersList.AddString(_T("3"));
	parametersList.AddString(_T("4"));
	parametersList.AddString(_T("5"));

	// Set Variables
	Drive = true;
	Arm = false;
	Claw = false;
	Sleep = false;
}


void CRobotControllerGUIDlg::OnBnClickedArm()
{
	// Clear Lists
	subCommandList.ResetContent();
	parametersList.ResetContent();

	// Add New Items
	subCommandList.AddString(_T("DOWN"));
	subCommandList.AddString(_T("UP"));

	// Set Variables
	Drive = false;
	Arm = true;
	Claw = false;
	Sleep = false;
}


void CRobotControllerGUIDlg::OnBnClickedClaw()
{
	// Clear Lists
	subCommandList.ResetContent();
	parametersList.ResetContent();

	// Add New Items
	subCommandList.AddString(_T("OPEN"));
	subCommandList.AddString(_T("CLOSE"));

	// Set Variables
	Drive = false;
	Arm = false;
	Claw = true;
	Sleep = false;
}


void CRobotControllerGUIDlg::OnBnClickedSleep()
{
	// Clear Lists
	subCommandList.ResetContent();
	parametersList.ResetContent();

	// Set Variables
	Drive = false;
	Arm = false;
	Claw = false;
	Sleep = true;
}

std::string CRobotControllerGUIDlg::getIPAddress()
{
	CString input;

	GetDlgItem(IDC_IPADDRESS)->GetWindowText(input);
	CT2CA converted(input);
	std::string ip(converted);

	return ip;
}

int CRobotControllerGUIDlg::getComPort()
{
	CString input;

	comPort.GetWindowTextW(input);

	CT2CA converted2(input);
	std::string com(converted2);
	int comP = std::stoi(com);

	return comP;
}

int CRobotControllerGUIDlg::getTelPort()
{
	CString input;

	GetDlgItem(IDC_TELEMETRYPORT)->GetWindowText(input);
	CT2CA converted(input);
	std::string tel(converted);
	int telP = std::stoi(tel);

	return telP;
}

int CRobotControllerGUIDlg::getSelectedSubCommand()
{
	return subCommandList.GetCurSel();
}

int CRobotControllerGUIDlg::getSelectedParameter()
{
	return parametersList.GetCurSel();
}

bool CRobotControllerGUIDlg::getDrive()
{
	return Drive;
}

bool CRobotControllerGUIDlg::getArm()
{
	return Arm;
}

bool CRobotControllerGUIDlg::getClaw()
{
	return Claw;
}

bool CRobotControllerGUIDlg::getSleep()
{
	return Sleep;
}

bool CRobotControllerGUIDlg::getButtonSend()
{
	return buttonSendIsClicked;
}

void CRobotControllerGUIDlg::populatePacketInfo(LPCTSTR pktCount, LPCTSTR Drive, LPCTSTR sonar, LPCTSTR armVal, LPCTSTR armStatus, LPCTSTR clawStatus)
{
	PacketInfo.InsertItem(0, LPCTSTR(pktCount));
	PacketInfo.SetItemText(0, 1, LPCTSTR(Drive));
	PacketInfo.SetItemText(0, 2, LPCTSTR(sonar));
	PacketInfo.SetItemText(0, 3, LPCTSTR(armVal));
	PacketInfo.SetItemText(0, 4, LPCTSTR(armStatus));
	PacketInfo.SetItemText(0, 5, LPCTSTR(clawStatus));
}

void CRobotControllerGUIDlg::populateRAWData(LPCTSTR data, LPCTSTR type)
{
	RAWData.InsertItem(0, data);
	RAWData.SetItemText(0, 1, type);
}

void CRobotControllerGUIDlg::populateSystemMessages(LPCTSTR msg)
{
	SystemMessages.InsertItem(0, msg);
}

void CRobotControllerGUIDlg::OnBnClickedConnectbutton()
{
	// TODO: Add your control notification handler code here
	buttonConnectIsClicked = true;
	std::thread(cmdThread, this).detach();
	std::thread(telThread, this).detach();
}


void CRobotControllerGUIDlg::OnBnClickedSendbutton()
{
	// TODO: Add your control notification handler code here
	buttonSendIsClicked = true;
}

void CRobotControllerGUIDlg::setButtonSendFalse()
{
	buttonSendIsClicked = false;
}

void CRobotControllerGUIDlg::setButtonConnectFalse()
{
	buttonConnectIsClicked = false;
}


void CRobotControllerGUIDlg::OnIpnFieldchangedIpaddress(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMIPADDRESS pIPAddr = reinterpret_cast<LPNMIPADDRESS>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;
}
