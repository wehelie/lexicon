#ifndef __NOTEDIALOG_H__
#define __NOTEDIALOG_H__
#include <QGridLayout>
#include <QFormLayout>
#include <QMenu>
#include <QMenuBar>
#include <QAction>
#include <QDebug>
#include <QLineEdit>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QPushButton>
#include <QSettings>
#include <QLabel>
#include <QToolButton>
#include <QDialog>
#include <QRadioButton>
#include <QGroupBox>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include "QsLog.h"
#include "place.h"
#include "notes.h"
#include "imedit.h"
#include "keyboardwidget.h"
#include "imeditor.h"
class ImLineEdit;
class NoteDialog : public QDialog {
  Q_OBJECT
 public:
  NoteDialog(const Place & p,QWidget * parent = 0);
  NoteDialog(Note *,QWidget * parent = 0);
  ~NoteDialog();
  QSize sizeHint() const;
  void setSubject(const QString &);
  void setModified(bool);
  bool isModified() const;
  QString getNote() { return m_note->edit()->toPlainText();}
  void setAutosave(bool v) { m_autosave = v;}
  enum { AttachSubject,AttachNote };
  public slots:
    void showOptions(bool);
    void showKeyboard();
    void cancel();
    void save();
    void print();
    void focusChanged(int);
    void onPrinterSetup();
 protected:
  void closeEvent(QCloseEvent *);
 private:
  void positionKeyboard();
  void setup();
  bool m_attached;
  bool m_changed;
  bool m_autosave;
  int m_attachedEdit;
  Place m_place;
  KeyboardWidget * m_keyboard;
  ImLineEdit * m_subject;
  ImEditor * m_note;
  QLineEdit * m_tags;
  QPushButton * m_keyboardButton;
  QPushButton * m_moreButton;
  QPushButton * m_printButton;
  QDialogButtonBox * m_moreButtonBox;
  QDialogButtonBox * m_buttonBox;
  QComboBox * m_type;
  Note * m_noteItem;
  QString m_subjectText;
  QString m_noteText;
  QString m_word;
  QRadioButton * m_typeUser;
  QRadioButton * m_typeSystem;
  int m_id;
  int m_noteType;
 signals:
  void saveNote(Note *);
  void noteSaved(bool);
};
#endif
