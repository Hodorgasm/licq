#ifndef ICQFUNCTIONS_H
#define ICQFUNCTIONS_H

#include <qdialog.h>

#include "history.h"

class QBoxLayout;
class QSplitter;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QButtonGroup;
class QRadioButton;
class QTextView;
class QTabWidget;
class QListViewItem;
class QTimer;
class QSpinBox;

class ICQUser;
class CICQDaemon;
class CEComboBox;
class ICQEvent;
class CICQEventTag;
class CInfoField;
class CSignalManager;
class MLEditWrap;
class MsgView;
class MLEditWrap;
class CUserEvent;

//=====ICQFunctions=============================================================

const int TAB_READ = 0;
const int TAB_SEND = 1;
const int TAB_GENERALINFO = 2;
const int TAB_MOREINFO = 3;
const int TAB_WORKINFO = 4;
const int TAB_ABOUT = 5;
const int TAB_HISTORY = 6;
const int TAB_SECURITY = 7;

struct STab
{
  QString label;
  QWidget *tab;
  bool loaded;
};

class ICQFunctions : public QWidget
{
   Q_OBJECT
public:
  ICQFunctions(CICQDaemon *s, CSignalManager *theSigMan,
               unsigned long _nUin, bool isAutoClose,
               QWidget *parent = 0, const char *name = 0);
  virtual ~ICQFunctions();
  void setupTabs(int);
  unsigned long Uin()  {  return m_nUin;  }

protected:
  unsigned long m_nUin;
  struct STab tabList[8];
  int currentTab;
  CICQDaemon *server;
  CSignalManager *sigman;
  QString m_sBaseTitle, m_sProgressMsg;
  CICQEventTag *icqEventTag;
  bool m_bOwner;
  QCheckBox *chkAutoClose;
  QTabWidget *tabs;
  QPushButton *btnSave, *btnOk, *btnCancel;

  // Read Event tab
  void CreateReadEventTab();
  QSplitter *splRead;
  MLEditWrap *mleRead;
  MsgView *msgView;
  QPushButton *btnRead1, *btnRead2, *btnRead3, *btnRead4;
  CUserEvent *m_xCurrentReadEvent;

  // Send Event tab
  void CreateSendEventTab();
  MLEditWrap *mleSend;
  QCheckBox *chkSendServer, *chkSpoof, *chkUrgent;
  QLineEdit *edtSpoof, *edtItem;
  QLabel *lblItem;
  QGroupBox *grpOpt;
  QButtonGroup *grpCmd;
  QRadioButton *rdbUrl, *rdbChat, *rdbFile, *rdbMsg;

  // User Info tab
  void CreateGeneralInfoTab();
  void InitGeneralInfoTab();
  CInfoField *nfoFirstName, *nfoLastName, *nfoEmail1, *nfoEmail2,
             *nfoAlias,*nfoStatus, *nfoIp, *nfoUin, *nfoCity, *nfoState,
             *nfoZipCode, *nfoAddress, *nfoCountry, *nfoFax, *nfoCellular,
             *nfoPhone, *nfoTimezone, *nfoLastOnline;
  CEComboBox *cmbCountry;
  QCheckBox *chkAuthorization;
  time_t m_nRemoteTimeOffset;
  QTimer *tmrTime;

  // More info
  void CreateMoreInfoTab();
  void InitMoreInfoTab();
  CInfoField *nfoAge, *nfoBirthday, *nfoLanguage[3], *nfoHomepage,
             *nfoGender;
  CEComboBox *cmbLanguage[3], *cmbGender;
  QSpinBox *spnBirthDay, *spnBirthMonth, *spnBirthYear;

  // Work info
  void CreateWorkInfoTab();
  void InitWorkInfoTab();
  CInfoField *nfoCompanyName, *nfoCompanyCity, *nfoCompanyState,
             *nfoCompanyAddress, *nfoCompanyPhone, *nfoCompanyFax,
             *nfoCompanyHomepage, *nfoCompanyPosition, *nfoCompanyDepartment;

  // About
  void CreateAboutTab();
  void InitAboutTab();
  QLabel *lblAbout;
  MLEditWrap *mleAbout;

  // History tab
  void CreateHistoryTab();
  void InitHistoryTab();
  CInfoField *nfoHistory;
  QTextView *mleHistory;
  QLabel *lblHistory;
  QCheckBox *chkHistoryReverse;
  HistoryList m_lHistoryList;
  HistoryListIter m_iHistorySIter;
  HistoryListIter m_iHistoryEIter;
  bool m_bHistoryReverse;
  unsigned short m_nHistoryIndex, m_nHistoryShowing;
  QPushButton *btnHistoryReload, *btnHistoryEdit;

  static unsigned short s_nX;
  static unsigned short s_nY;

  virtual void keyPressEvent(QKeyEvent *);
  virtual void closeEvent(QCloseEvent *);
  void SetInfo(ICQUser *);
  void SetGeneralInfo(ICQUser *);
  void SetMoreInfo(ICQUser *);
  void SetWorkInfo(ICQUser *);
  void SetAbout(ICQUser *);
  void SaveGeneralInfo();
  void SaveMoreInfo();
  void SaveWorkInfo();
  void SaveAbout();
  void saveHistory();
  void generateReply();
  void SetupHistory();
  void ShowHistory();

public slots:
  void SendUrl(const char *url, const char *desc);
  void SendFile(const char *file, const char *desc);

protected slots:
   void callFcn();
   void doneFcn(ICQEvent *);
   void slot_updatedUser(unsigned long, unsigned long);
   void tabSelected(const QString &);
   void ShowHistoryPrev();
   void ShowHistoryNext();
   void printMessage(QListViewItem *);
   void save();
   void setSpoofed();
   void specialFcn(int);
   void slot_historyReverse(bool);
   void slot_historyReload();
   void slot_historyEdit();
   void slot_updatetime();
   void slot_readbtn1();
   void slot_readbtn2();
   void slot_readbtn3();
   void slot_readbtn4();
   void slot_aboutToShow(QWidget *);

signals:
   void signal_updatedUser(unsigned long, unsigned long);
   void signal_finished(unsigned long);
};


#endif
