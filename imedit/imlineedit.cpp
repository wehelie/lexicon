#include "imlineedit.h"
ImLineEdit::ImLineEdit(QWidget * parent)
  : QLineEdit(parent)
{
  m_mapper = im_new();
  m_prev_char = 0;
  m_debug = false;
}
ImLineEdit::~ImLineEdit() {
  im_free(m_mapper);
}
bool ImLineEdit::loadMap(const QString & filename,const QString & mapname) {
  QFile f(filename);
  if (!f.open(QIODevice::ReadOnly)) {
    // TODO emit(logMessage(QString("Error loading file %1: %2 ").arg(fileName).arg(f.errorString())));
    return false;
  }
  if (mapname.isEmpty()) {
    im_load_map_from_json(m_mapper,filename.toUtf8().constData(),0);
  }
  else {
    im_load_map_from_json(m_mapper,filename.toUtf8().constData(),mapname.toUtf8().constData());
  }
  return true;
}
void ImLineEdit::activateMap(const QString & name,bool activate) {
  if (! activate ) {
    m_activeMap.clear();
    return;
  }
  if (m_mapper->hasMap(name) && activate) {
    m_activeMap = name;
  }
}
void ImLineEdit::readSettings() {
}
void ImLineEdit::keyPressEvent(QKeyEvent * event) {
  ushort pc;
  //  qDebug() << "keypress" << mapEnabled << mapname;;
  if (event->modifiers() & Qt::ControlModifier) {
    return QLineEdit::keyPressEvent(event);
  }
  if (m_debug) {
    QString t;
    QTextStream out(&t);
    out << "ImEdit in: 0x" << qSetFieldWidth(4) << qSetPadChar(QChar('0')) << hex << event->key() << " " << UcdScripts::getScript(event->key());
    out.reset();
    out << " " << event->text();

  }
  if ( m_activeMap.isEmpty()) {
    return QLineEdit::keyPressEvent(event);
  }

  //event->text().toUtf8().data());
  const QChar * uc = event->text().unicode();
  pc = uc->unicode();
  if (pc == 0) {
    if (m_debug) {
      /// TODO error handling
    }
    return QLineEdit::keyPressEvent(event);
  }
  im_char * cc = im_convert(this->m_mapper,this->m_activeMap.toUtf8().constData(),uc->unicode(),m_prev_char);
  if (cc->processed) {
    event->ignore();
    QKeyEvent * nevent = new QKeyEvent(QEvent::KeyPress, cc->uc, Qt::NoModifier,cc->c);
    m_prev_char = cc->uc;
    QLineEdit::keyPressEvent(nevent);
    if (m_debug) {
      QString t;
      QTextStream out(&t);
      out << "ImLineEdit out: 0x" << qSetFieldWidth(4) << qSetPadChar(QChar('0')) << hex << nevent->key();
      out.reset();
      out << nevent->text();

    }
    //     QApplication::postEvent(event->target, nevent);
    return;
  }
  else {
    m_prev_char = pc;
    QLineEdit::keyPressEvent(event);
  }
}
