// albert - a simple application launcher for linux
// Copyright (C) 2014 Manuel Schneider
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "hotkeymanager_win.h"
#include "windows.h"

namespace {

static int sid = 0;
QSet<int> sGrabbedIds;

struct Qt_VK_Keymap
{
    int key;
    UINT vk;
};

static Qt_VK_Keymap Qt_VK_table[] = { // TODO make this hold groups too e.g. META r+l
        { Qt::Key_Escape,      VK_ESCAPE },
        { Qt::Key_Tab,         VK_TAB },
        { Qt::Key_Backtab,     0 },
        { Qt::Key_Backspace,   VK_BACK },
        { Qt::Key_Return,      VK_RETURN },
        { Qt::Key_Enter,       VK_RETURN },
        { Qt::Key_Insert,      VK_INSERT },
        { Qt::Key_Delete,      VK_DELETE },
        { Qt::Key_Pause,       VK_PAUSE },
        { Qt::Key_Print,       VK_SNAPSHOT },
        { Qt::Key_SysReq,      0 },
        { Qt::Key_Clear,       VK_CLEAR },
        { Qt::Key_Home,        VK_HOME },
        { Qt::Key_End,         VK_END },
        { Qt::Key_Left,        VK_LEFT },
        { Qt::Key_Up,          VK_UP },
        { Qt::Key_Right,       VK_RIGHT },
        { Qt::Key_Down,        VK_DOWN },
        { Qt::Key_PageUp,      VK_PRIOR },
        { Qt::Key_PageDown,    VK_NEXT },
        { Qt::Key_Shift,       VK_SHIFT },
        { Qt::Key_Control,     VK_CONTROL },
        { Qt::Key_Meta,        VK_LWIN },
        { Qt::Key_Alt,         VK_MENU },
        { Qt::Key_CapsLock,    VK_CAPITAL },
        { Qt::Key_NumLock,     VK_NUMLOCK },
        { Qt::Key_ScrollLock,  VK_SCROLL },
        { Qt::Key_F1,          VK_F1 },
        { Qt::Key_F2,          VK_F2 },
        { Qt::Key_F3,          VK_F3 },
        { Qt::Key_F4,          VK_F4 },
        { Qt::Key_F5,          VK_F5 },
        { Qt::Key_F6,          VK_F6 },
        { Qt::Key_F7,          VK_F7 },
        { Qt::Key_F8,          VK_F8 },
        { Qt::Key_F9,          VK_F9 },
        { Qt::Key_F10,         VK_F10 },
        { Qt::Key_F11,         VK_F11 },
        { Qt::Key_F12,         VK_F12 },
        { Qt::Key_F13,         VK_F13 },
        { Qt::Key_F14,         VK_F14 },
        { Qt::Key_F15,         VK_F15 },
        { Qt::Key_F16,         VK_F16 },
        { Qt::Key_F17,         VK_F17 },
        { Qt::Key_F18,         VK_F18 },
        { Qt::Key_F19,         VK_F19 },
        { Qt::Key_F20,         VK_F20 },
        { Qt::Key_F21,         VK_F21 },
        { Qt::Key_F22,         VK_F22 },
        { Qt::Key_F23,         VK_F23 },
        { Qt::Key_F24,         VK_F24 },
        { Qt::Key_F25,         0 },
        { Qt::Key_F26,         0 },
        { Qt::Key_F27,         0 },
        { Qt::Key_F28,         0 },
        { Qt::Key_F29,         0 },
        { Qt::Key_F30,         0 },
        { Qt::Key_F31,         0 },
        { Qt::Key_F32,         0 },
        { Qt::Key_F33,         0 },
        { Qt::Key_F34,         0 },
        { Qt::Key_F35,         0 },
        { Qt::Key_Super_L,     0 },
        { Qt::Key_Super_R,     0 },
        { Qt::Key_Menu,        0 },
        { Qt::Key_Hyper_L,     0 },
        { Qt::Key_Hyper_R,     0 },
        { Qt::Key_Help,        0 },
        { Qt::Key_Direction_L, 0 },
        { Qt::Key_Direction_R, 0 },

        { Qt::Key_QuoteLeft,   VK_OEM_8 },
        { Qt::Key_Minus,       VK_OEM_MINUS },
        { Qt::Key_Equal,       VK_OEM_PLUS },

        { Qt::Key_BracketLeft, VK_OEM_4 },
        { Qt::Key_BracketRight,VK_OEM_6 },

        { Qt::Key_Semicolon,   VK_OEM_1 },
        { Qt::Key_Apostrophe,  VK_OEM_3 },
        { Qt::Key_NumberSign,  VK_OEM_7 },

        { Qt::Key_Backslash,   VK_OEM_5 },
        { Qt::Key_Comma,	   VK_OEM_COMMA },
        { Qt::Key_Period,      VK_OEM_PERIOD },
        { Qt::Key_Slash,       VK_OEM_2 },

        { Qt::Key_unknown,     0 },
};

}


/**************************************************************************/
/**************************************************************************/
/**************************************************************************/
HotkeyManagerPrivate::HotkeyManagerPrivate(QObject *parent)
    : QObject(parent)
{

    QAbstractEventDispatcher::instance()->installNativeEventFilter(this);
}

/**************************************************************************/
bool HotkeyManagerPrivate::registerNativeHotkey(const int hk)
{
    int keyQt = hk & ~Qt::KeyboardModifierMask;
    int modQt = hk &  Qt::KeyboardModifierMask;


    /* Translate key symbol ( Qt -> X ) */
    UINT key = 0;
    if (keyQt == 0x20 || //SPACE
            (keyQt >= 0x30 && keyQt <= 0x39) || // NUMBRS
            (keyQt > 0x41 && keyQt <= 0x5a) || // LETTERS
            (keyQt > 0x60 && keyQt <= 0x7a))   //NUMPAD
        key = keyQt;
    else {
        // Others require lookup from a keymap
        for (int n = 0; Qt_VK_table[n].key != Qt::Key_unknown; ++n) {
            if (Qt_VK_table[n].key == keyQt) {
                key = Qt_VK_table[n].vk;
                break;
            }
        }
        if (!key)
            return false;
    }


    /* Translate modifiers ( Qt -> X ) */

    unsigned int mods = 0; // MOD_NOREPEAT;
    if (modQt & Qt::META)
            mods |= MOD_WIN;
    if (modQt & Qt::SHIFT)
            mods |= MOD_SHIFT;
    if (modQt & Qt::CTRL)
            mods |= MOD_CONTROL;
    if (modQt & Qt::ALT)
            mods |= MOD_ALT;

    /* Grab the key combo*/
    bool success;
    success = RegisterHotKey(NULL, sid, mods, key);
    if (success)
        sGrabbedIds.insert(sid++);
    return success;
}

/**************************************************************************/
void HotkeyManagerPrivate::unregisterNativeHotkeys()
{
    for ( int i : sGrabbedIds)
        UnregisterHotKey(NULL, i);
}

/**************************************************************************/
bool HotkeyManagerPrivate::nativeEventFilter(const QByteArray &eventType, void *message, long *result)
{
    if (eventType == "windows_generic_MSG") {
        MSG* msg = static_cast<MSG *>(message);
        if (msg->message == WM_HOTKEY)
        {
            // Check if the key is one of the registered
            for (int i : sGrabbedIds)
                if (msg->wParam == i)
                {
                    emit hotKeyPressed();
                    return true;
                }
        }
    }
    return false;
}


















#include "qxtglobalshortcut_p.h"

#include <QHash>
#include <QMap>

#include <Carbon/Carbon.h>

typedef QPair<uint, uint> Identifier;
static QMap<quint32, EventHotKeyRef> keyRefs;
static QHash<Identifier, quint32> keyIDs;
static quint32 hotKeySerial = 0;
static bool qxt_mac_handler_installed = false;

OSStatus qxt_mac_handle_hot_key(EventHandlerCallRef nextHandler, EventRef event, void *data)
{
    Q_UNUSED(nextHandler)
    Q_UNUSED(data)
    if (GetEventClass(event) == kEventClassKeyboard && GetEventKind(event) == kEventHotKeyPressed) {
        EventHotKeyID keyID;
        GetEventParameter(event, kEventParamDirectObject, typeEventHotKeyID, NULL, sizeof(keyID), NULL, &keyID);
        Identifier id = keyIDs.key(keyID.id);
        QxtGlobalShortcutPrivate::activateShortcut(id.second, id.first);
    }

    return noErr;
}

bool QxtGlobalShortcutPrivate::nativeEventFilter(const QByteArray &eventType,
                                                 void *message, long *result)
{
    Q_UNUSED(eventType)
    Q_UNUSED(message)
    Q_UNUSED(result)
    return false;
}

quint32 QxtGlobalShortcutPrivate::nativeModifiers(Qt::KeyboardModifiers modifiers)
{
    quint32 native = 0;
    if (modifiers & Qt::ShiftModifier)
        native |= shiftKey;
    if (modifiers & Qt::ControlModifier)
        native |= cmdKey;
    if (modifiers & Qt::AltModifier)
        native |= optionKey;
    if (modifiers & Qt::MetaModifier)
        native |= controlKey;
    if (modifiers & Qt::KeypadModifier)
        native |= kEventKeyModifierNumLockMask;
    return native;
}

quint32 QxtGlobalShortcutPrivate::nativeKeycode(Qt::Key key)
{
    UTF16Char ch;
    // Constants found in NSEvent.h from AppKit.framework
    switch (key) {
    case Qt::Key_Return:
        return kVK_Return;
    case Qt::Key_Enter:
        return kVK_ANSI_KeypadEnter;
    case Qt::Key_Tab:
        return kVK_Tab;
    case Qt::Key_Space:
        return kVK_Space;
    case Qt::Key_Backspace:
        return kVK_Delete;
    case Qt::Key_Control:
        return kVK_Command;
    case Qt::Key_Shift:
        return kVK_Shift;
    case Qt::Key_CapsLock:
        return kVK_CapsLock;
    case Qt::Key_Option:
        return kVK_Option;
    case Qt::Key_Meta:
        return kVK_Control;
    case Qt::Key_F17:
        return kVK_F17;
    case Qt::Key_VolumeUp:
        return kVK_VolumeUp;
    case Qt::Key_VolumeDown:
        return kVK_VolumeDown;
    case Qt::Key_F18:
        return kVK_F18;
    case Qt::Key_F19:
        return kVK_F19;
    case Qt::Key_F20:
        return kVK_F20;
    case Qt::Key_F5:
        return kVK_F5;
    case Qt::Key_F6:
        return kVK_F6;
    case Qt::Key_F7:
        return kVK_F7;
    case Qt::Key_F3:
        return kVK_F3;
    case Qt::Key_F8:
        return kVK_F8;
    case Qt::Key_F9:
        return kVK_F9;
    case Qt::Key_F11:
        return kVK_F11;
    case Qt::Key_F13:
        return kVK_F13;
    case Qt::Key_F16:
        return kVK_F16;
    case Qt::Key_F14:
        return kVK_F14;
    case Qt::Key_F10:
        return kVK_F10;
    case Qt::Key_F12:
        return kVK_F12;
    case Qt::Key_F15:
        return kVK_F15;
    case Qt::Key_Help:
        return kVK_Help;
    case Qt::Key_Home:
        return kVK_Home;
    case Qt::Key_PageUp:
        return kVK_PageUp;
    case Qt::Key_Delete:
        return kVK_ForwardDelete;
    case Qt::Key_F4:
        return kVK_F4;
    case Qt::Key_End:
        return kVK_End;
    case Qt::Key_F2:
        return kVK_F2;
    case Qt::Key_PageDown:
        return kVK_PageDown;
    case Qt::Key_F1:
        return kVK_F1;
    case Qt::Key_Left:
        return kVK_LeftArrow;
    case Qt::Key_Right:
        return kVK_RightArrow;
    case Qt::Key_Down:
        return kVK_DownArrow;
    case Qt::Key_Up:
        return kVK_UpArrow;
    default:
        ;
    }

    if (key == Qt::Key_Escape)
        ch = 27;
    else if (key == Qt::Key_Return)
        ch = 13;
    else if (key == Qt::Key_Enter)
        ch = 3;
    else if (key == Qt::Key_Tab)
        ch = 9;
    else
        ch = key;

    CFDataRef currentLayoutData;
    TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();

    if (currentKeyboard == NULL)
        return 0;

    currentLayoutData = (CFDataRef)TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData);
    CFRelease(currentKeyboard);
    if (currentLayoutData == NULL)
        return 0;

    UCKeyboardLayout *header = (UCKeyboardLayout *)CFDataGetBytePtr(currentLayoutData);
    UCKeyboardTypeHeader *table = header->keyboardTypeList;

    uint8_t *data = (uint8_t *)header;
    // God, would a little documentation for this shit kill you...
    for (quint32 i = 0; i < header->keyboardTypeCount; ++i) {
        UCKeyStateRecordsIndex *stateRec = 0;
        if (table[i].keyStateRecordsIndexOffset != 0) {
            stateRec = reinterpret_cast<UCKeyStateRecordsIndex *>(data + table[i].keyStateRecordsIndexOffset);
            if (stateRec->keyStateRecordsIndexFormat != kUCKeyStateRecordsIndexFormat) stateRec = 0;
        }

        UCKeyToCharTableIndex *charTable = reinterpret_cast<UCKeyToCharTableIndex *>(data + table[i].keyToCharTableIndexOffset);
        if (charTable->keyToCharTableIndexFormat != kUCKeyToCharTableIndexFormat)
            continue;

        for (quint32 j = 0; j < charTable->keyToCharTableCount; ++j) {
            UCKeyOutput *keyToChar = reinterpret_cast<UCKeyOutput *>(data + charTable->keyToCharTableOffsets[j]);
            for (quint32 k = 0; k < charTable->keyToCharTableSize; ++k) {
                if (keyToChar[k] & kUCKeyOutputTestForIndexMask) {
                    long idx = keyToChar[k] & kUCKeyOutputGetIndexMask;
                    if (stateRec && idx < stateRec->keyStateRecordCount) {
                        UCKeyStateRecord *rec = reinterpret_cast<UCKeyStateRecord *>(data + stateRec->keyStateRecordOffsets[idx]);
                        if (rec->stateZeroCharData == ch) return k;
                    }
                } else if (!(keyToChar[k] & kUCKeyOutputSequenceIndexMask) && keyToChar[k] < 0xFFFE) {
                    if (keyToChar[k] == ch)
                        return k;
                }
            } // for k
        } // for j
    } // for i
    return 0;
}

bool QxtGlobalShortcutPrivate::registerShortcut(quint32 nativeKey, quint32 nativeMods)
{
    if (!qxt_mac_handler_installed) {
        EventTypeSpec t;
        t.eventClass = kEventClassKeyboard;
        t.eventKind = kEventHotKeyPressed;
        InstallApplicationEventHandler(&qxt_mac_handle_hot_key, 1, &t, NULL, NULL);
    }

    EventHotKeyID keyID;
    keyID.signature = 'cute';
    keyID.id = ++hotKeySerial;

    EventHotKeyRef ref = 0;
    bool rv = !RegisterEventHotKey(nativeKey, nativeMods, keyID, GetApplicationEventTarget(), 0, &ref);
    if (rv) {
        keyIDs.insert(Identifier(nativeMods, nativeKey), keyID.id);
        keyRefs.insert(keyID.id, ref);
    }
    return rv;
}

bool QxtGlobalShortcutPrivate::unregisterShortcut(quint32 nativeKey, quint32 nativeMods)
{
    Identifier id(nativeMods, nativeKey);
    if (!keyIDs.contains(id))
        return false;

    EventHotKeyRef ref = keyRefs.take(keyIDs[id]);
    keyIDs.remove(id);
    return !UnregisterEventHotKey(ref);
}













