#include "notedialog.h"
#include "laneslexicon.h"
extern LanesLexicon * getApp();
Note::Note() {
  m_id = -1;
}
void Note::save(QSqlDatabase * db) {
  qDebug() << Q_FUNC_INFO;
  if (m_id == -1) {
    QSqlQuery addQuery(*db);

    if (! addQuery.prepare("insert into notes (datasource,word,place,subject,note,created) \
           values (:datasource,:word,:place,:subject,:note,:created)")) {
      QLOG_WARN() << "SQL prepare error" << addQuery.lastError().text();
      return;
    }
    qDebug() << "note:" << m_note;
    addQuery.bindValue(":datasource",m_place.getSource());
    addQuery.bindValue(":word",m_place.getWord());
    addQuery.bindValue(":place",m_place.toString());
    addQuery.bindValue(":subject",m_subject);
    addQuery.bindValue(":note",m_note);
    addQuery.bindValue(":created",QDateTime::currentDateTime().toString());
    if (! addQuery.exec()) {
      QLOG_WARN() << "SQL exec error" << addQuery.lastError().text();
      return;
    }

    //      addQuery.bindValue(
  }
}
NoteMaster::NoteMaster() {
  readSettings();
  openDb();
}
void NoteMaster::save(Note * n) {

}
bool NoteMaster::openDb() {
  qDebug() << Q_FUNC_INFO;
  if ( ! m_enabled )
    return false;
  if ( ! m_db.isOpen()) {
    QFile dbfile(m_dbName);
    if (! dbfile.exists()) {
      /// TODO maybe create database
      m_enabled = false;
      QLOG_WARN() << "Cannot find notes database" << m_dbName;
      return false;
    }
  }
  else {
    return true;
  }
  m_db = QSqlDatabase::addDatabase("QSQLITE","notesdb");
  m_db.setDatabaseName(m_dbName);
  if (! m_db.open()) {
    QMessageBox::critical(0,QObject::tr("Database Error"),
                          m_db.lastError().text());
    m_enabled = false;
    return false;
  }
  else {
    qDebug() << "tables" << m_db.tables();
  }

  addQuery = QSqlQuery(m_db);
  if (! addQuery.prepare("insert into notes (datasource,word,place,subject,note,created) \
           values (:datasource,:word,:place,:subject,:note,:created)")) {
    QLOG_WARN() << "SQL add prepare error" << addQuery.lastError().text();
    m_enabled = false;
    return false;
  }
  updateQuery = QSqlQuery(m_db);
  if (! updateQuery.prepare("update notes set subject = ?, note = ? where id = ?")) {
    QLOG_WARN() << "SQL update prepare error" << updateQuery.lastError().text();
    m_enabled = false;
    return false;
  }
  deleteQuery = QSqlQuery(m_db);
  if (! deleteQuery.prepare("delete from  notes  where id = ?")) {
    QLOG_WARN() << "SQL delete error" << deleteQuery.lastError().text();
    m_enabled = false;
    return false;
  }
  findQuery = QSqlQuery(m_db);
  if (! findQuery.prepare("select id,word,place,subject,note,created from notes where word = ?")) {
    QLOG_WARN() << "SQL find error" << findQuery.lastError().text();
    m_enabled = false;
    return false;
  }
  qDebug() << "=====================================";
  qDebug() << "Notes system successfully initialised";
  qDebug() << "=====================================";
  return true;
}
QList<Note *> NoteMaster::find(const QString & word) {
  findQuery.bindValue(0,word);
  findQuery.exec();
  QList<Note *> notes;
  while(findQuery.next()) {
    Note * n = new Note();
    n->setId(findQuery.value(0).toInt());
    n->setWord(findQuery.value(1).toString());
    n->setPlace(Place::fromString(findQuery.value(2).toString()));
    n->setSubject(findQuery.value(3).toString());
    n->setNote(findQuery.value(4).toString());
    n->setWhen(findQuery.value(5).toString());
    notes << n;
  }
  qDebug() << Q_FUNC_INFO << word << "find count" << notes.size();
  return notes;
}
void NoteMaster::readSettings() {
  Lexicon * app = qobject_cast<Lexicon *>(qApp);
  QSettings * settings = app->getSettings();
  settings->setIniCodec("UTF-8");
  settings->beginGroup("Notes");
  m_dbName = settings->value("Database","notes.sqlite").toString();
  m_autosave = settings->value("Autosave",true).toBool();
  m_enabled = settings->value("Enabled",true).toBool();

}
NoteDialog::NoteDialog(const Place & p,QWidget * parent) : QDialog(parent) {
  m_data.setPlace(p);
  m_place = p;
  QVBoxLayout * layout = new QVBoxLayout;
  m_subject = new QLineEdit(this);
  m_subject->setText(p.getShortText());
  m_note = new QTextEdit(this);

  layout->addWidget(m_subject);
  layout->addWidget(m_note);
  setLayout(layout);
  if (! m_subject->text().isEmpty()) {
    m_note->setFocus();
  }
  //  setWindowFlags(Qt::CustomizeWindowHint);
  setSizeGripEnabled(true);
  m_changed = false;
  m_autosave = false;
  connect(this,SIGNAL(saveNote(Note *)),getApp(),SLOT(saveNote(Note *)));
}
void NoteDialog::setModified(bool v) {
  m_changed = v;
  m_note->document()->setModified(v);
}
QSize NoteDialog::sizeHint() const {
  QSize sz(300,300);
  return sz;
}
void NoteDialog::closeEvent(QCloseEvent * event) {
  qDebug() << Q_FUNC_INFO << "modified" << this->isModified();
  if (this->isModified()) {
    if (m_autosave) {
        Note * n = new Note();
        n->setSubject(m_subject->text());
        n->setNote(m_note->toPlainText());
        n->setPlace(m_place);
        emit(saveNote(n));
    }
    else {
      QMessageBox msgBox;
      msgBox.setText("The document has been modified.");
      msgBox.setInformativeText("Do you want to save your changes?");
      msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
      msgBox.setDefaultButton(QMessageBox::Save);
      msgBox.setWindowModality(Qt::ApplicationModal);
      int ret = msgBox.exec();
      if (ret == QMessageBox::Save) {
        Note * n = new Note();
        n->setSubject(m_subject->text());
        n->setNote(m_note->toPlainText());
        n->setPlace(m_place);
        emit(saveNote(n));
      }
    }
  }
  QDialog::closeEvent(event);
}
NoteDialog::~NoteDialog() {
  qDebug() << Q_FUNC_INFO;
}
void NoteDialog::setSubject(const QString & text) {
  m_subject->setText(text);
  m_data.setSubject(text);
  m_changed = true;
}
bool NoteDialog::isModified() const {
  if (m_changed || m_note->document()->isModified())
    return true;

  return false;
}