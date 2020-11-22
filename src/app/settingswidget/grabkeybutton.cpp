// Copyright (C) 2014-2018 Manuel Schneider

#include "grabkeybutton.h"
#include <QDebug>

QString modifiersToQString(int mods) {
    // On macOS, Key_Control corresponds to the Command keys.
    // On macOS, Key_Meta corresponds to the Control keys.
    // On Windows keyboards, this key is mapped to the Windows key.
#if defined __APPLE__
    QString s;
    if (mods & Qt::ShiftModifier)
        s.append("⇧");
    if (mods & Qt::ControlModifier)
        s.append("⌘");
    if (mods & Qt::AltModifier)
        s.append("⌥");
    if (mods & Qt::MetaModifier)
        s.append("⌃");
    return s;
#elif defined _WIN32
    QKeySequence(mods|Qt::Key_Question).toString()
    throw("Not implemented!")
#else
    QKeySequence((mods&~Qt::GroupSwitchModifier)|Qt::Key_Question).toString()  //QTBUG-45568
#endif
}



Core::GrabKeyButton::GrabKeyButton(QWidget * parent) : QPushButton(parent) {
    waitingForHotkey_ = false;
    connect(this, &QPushButton::clicked,
            this, &GrabKeyButton::onClick);
}
Core::GrabKeyButton::~GrabKeyButton() { }

/** ***************************************************************************/
void Core::GrabKeyButton::onClick() {
    oldText_ = text();
    setText("?");
    grabAll();
}

/** ***************************************************************************/
void Core::GrabKeyButton::grabAll() {
    grabKeyboard();
    grabMouse();
    waitingForHotkey_ = true;
}

/** ***************************************************************************/
void Core::GrabKeyButton::releaseAll() {
    releaseKeyboard();
    releaseMouse();
    waitingForHotkey_ = false;
}

/** ***************************************************************************/
void Core::GrabKeyButton::keyPressEvent(QKeyEvent *event) {
    if ( waitingForHotkey_ ) {
        // Modifier pressed -> update the label
        int key = event->key();
        int mods = event->modifiers();

        if(key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Super_L || key == Qt::Key_Super_R || key == Qt::Key_Meta ) {
            qDebug() << QKeySequence(mods).toString() << modifiersToQString(mods);
            setText(modifiersToQString(mods));
            event->accept();
            return;
        }

        if(key == Qt::Key_Escape && mods == Qt::NoModifier) {
            event->accept();
            setText(oldText_);
            releaseAll(); // Can not be before since window closes on esc
            return;
        }
        releaseAll();

        setText(modifiersToQString(mods).append("+").append(QKeySequence(key).toString()));
        emit keyCombinationPressed(mods|key);
        return;
    }
//    QWidget::keyPressEvent( event );
}

/** ***************************************************************************/
void Core::GrabKeyButton::keyReleaseEvent(QKeyEvent *event) {
    if ( waitingForHotkey_ ) {
        // Modifier released -> update the label
        int key = event->key();
        if(key == Qt::Key_Control || key == Qt::Key_Shift || key == Qt::Key_Alt || key == Qt::Key_Super_L || key == Qt::Key_Super_R || key == Qt::Key_Meta) {
            int mods = event->modifiers();
            qDebug() << QKeySequence(mods).toString() << modifiersToQString(mods);
            setText(modifiersToQString(mods).append("+?"));
            event->accept();
            return;
        }
        return;
    }
    QWidget::keyReleaseEvent( event );
}
