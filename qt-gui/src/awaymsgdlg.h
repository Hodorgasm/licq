#ifndef AWAYMSG_H
#define AWAYMSG_H

#include <qdialog.h>

class QPushButton;
class QComboBox;

class MLEditWrap;

class AwayMsgDlg : public QDialog
{
  Q_OBJECT
public:
  AwayMsgDlg(QWidget *parent = 0, const char *name = 0);
  void SelectAutoResponse(unsigned short status);
  virtual void hide();

protected:
  MLEditWrap *mleAwayMsg;
  static int s_nX, s_nY;
  QPushButton* btnSelect;
  unsigned short m_nStatus;
  short m_nSAR;

protected slots:
  void ok();
  void show();
  void slot_selectMessage();
};


#endif
