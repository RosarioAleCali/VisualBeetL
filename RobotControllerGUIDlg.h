
// RobotControllerGUIDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <string>
#include <sstream>
#include <thread>

// CRobotControllerGUIDlg dialog
class CRobotControllerGUIDlg : public CDialogEx
{
// Construction
public:
	CRobotControllerGUIDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ROBOTCONTROLLERGUI_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	CIPAddressCtrl IPAddress;
	CEdit comPort;
	CEdit telPort;
	CComboBox subCommandList;
	CComboBox parametersList;
	bool Drive;
	bool Arm;
	bool Claw;
	bool Sleep;
	bool buttonConnectIsClicked;
	bool buttonSendIsClicked;
	CListCtrl PacketInfo;
	CListCtrl RAWData;
	CListCtrl SystemMessages;
public:
	afx_msg void OnBnClickedDrive();
	afx_msg void OnBnClickedArm();
	afx_msg void OnBnClickedClaw();
	afx_msg void OnBnClickedSleep();
	std::string getIPAddress();
	int getComPort();
	int getTelPort();
	int getSelectedSubCommand();
	int getSelectedParameter();
	bool getDrive();
	bool getArm();
	bool getClaw();
	bool getSleep();
	bool getButtonSend();
	void populatePacketInfo(LPCTSTR pktCount, LPCTSTR Drive, LPCTSTR sonar, LPCTSTR armVal, LPCTSTR armStatus, LPCTSTR clawStatus);
	void populateRAWData(LPCTSTR data, LPCTSTR type);
	void populateSystemMessages(LPCTSTR msg);
	afx_msg void OnBnClickedConnectbutton();
	afx_msg void OnBnClickedSendbutton();
	void setButtonSendFalse();
	void setButtonConnectFalse();
	afx_msg void OnIpnFieldchangedIpaddress(NMHDR *pNMHDR, LRESULT *pResult);
};
