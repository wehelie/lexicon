#ifndef __HISTORY_H__
#define __HISTORY_H__
#include <QtWidgets>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include "QsLog.h"

class HistoryEvent {
 public:
  HistoryEvent();
  void setWord(const QString & word) { m_word = word;}
  void setRoot(const QString & word) { m_root = word;}
  void setNode(const QString & word) { m_node = word;}
  void setWhen(const QDateTime & word) { m_when = word;}
  void setId(const int i) { m_id = i;}
  QString getWord() { return m_word;}
  QString getRoot() { return m_root;}
  QString getNode() { return m_node;}
  QDateTime getWhen() { return m_when;}
  int getId() { return m_id;}
  bool isValid() { return m_id == -1;}
  bool matches(HistoryEvent *);
 private:
  int m_id;
  QString m_word;
  QString m_root;
  QString m_node;
  QDateTime m_when;
};

class HistoryMaster {
 public:
  HistoryMaster(const QString & dbname);
  ~HistoryMaster();
  //  bool add(const QString & root,const QString & word, const QString & node);
  HistoryEvent * getEvent(int id);
  bool add(HistoryEvent *);
  void on() { m_historyOn = true;}
  void off() { m_historyOn = false;}
  bool isOn() { return m_historyOn;}
  QList<HistoryEvent *> getHistory(int howMany,int direction,int startPos = -1);
 private:
  int m_lastId;
  bool openDatabase(const QString & dbname);
  bool m_historyOn;
  QSqlDatabase m_db;
  bool m_historyOk;
  QSqlQuery * m_getQuery;
  QSqlQuery * m_addQuery;
  QSqlQuery * m_backQuery;
  QSqlQuery * m_forQuery;
};
extern HistoryMaster * getHistory();
#endif
