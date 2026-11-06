/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2006
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * "Software"), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 * LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 * OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#include <iostream>
#include <gecode/gist/textoutput.hh>
#include <gecode/gist/gecodelogo.hh>

namespace Gecode::Gist {

    /// \brief An outputstream that prints on a QTextEdit
    class GistOutputStream : public std::basic_ostream<char, std::char_traits<char>> {
        /// \brief Buffer for printing on a QTextEdit
        class Buf : public std::basic_streambuf<char, std::char_traits<char>> {
            QString buffer;
            QTextEdit* editor;

        public:
            void
            flush() {
                QTextBlockFormat bf = editor->textCursor().blockFormat();
                bf.setBottomMargin(0);
                editor->textCursor().setBlockFormat(bf);
                editor->append(buffer);
                buffer.clear();
            }
            Buf(QTextEdit* e) : editor(e) {}

        protected:
            int
            overflow(int v = std::char_traits<char>::eof()) override {
                if (v == '\n') {
                    flush();
                } else {
                    buffer += static_cast<char>(v);
                }
                return v;
            }
        };

        Buf _buf;

    public:
        GistOutputStream(QTextEdit* editor) : std::basic_ostream<char, std::char_traits<char>>(&_buf), _buf(editor) {
            clear();
        }
        void
        flush() {
            _buf.flush();
        }
    };

    TextOutputI::TextOutputI(const std::string& name, QWidget* parent) : QMainWindow(parent) {
        Logos logos;

        QPixmap my_pic;
        my_pic.loadFromData(logos.gist_logo, logos.gist_logo_size);
        setWindowIcon(my_pic);

        QFont font;
        QString font_family("Courier");
        font.setFamily(font_family);
        font.setFixedPitch(true);
        font.setPointSize(12);

        editor = new QTextEdit;
        editor->setFont(font);
        editor->setReadOnly(true);
        editor->setLineWrapMode(QTextEdit::FixedColumnWidth);
        editor->setLineWrapColumnOrWidth(80);
        os = new GistOutputStream(editor);

        QAction* clear_text = new QAction("Clear", this);
        clear_text->setShortcut(QKeySequence("Ctrl+K"));
        this->addAction(clear_text);
        connect(clear_text, SIGNAL(triggered()), editor, SLOT(clear()));

        QAction* close_window = new QAction("Close window", this);
        close_window->setShortcut(QKeySequence("Ctrl+W"));
        this->addAction(close_window);
        connect(close_window, SIGNAL(triggered()), this, SLOT(close()));

        QToolBar* t = addToolBar("Tools");
        t->setFloatable(false);
        t->setMovable(false);
        t->addAction(clear_text);

        stay_on_top = new QAction("Stay on top", this);
        stay_on_top->setCheckable(true);
        t->addAction(stay_on_top);
        connect(stay_on_top, SIGNAL(changed()), this, SLOT(change_stay_on_top()));

        change_stay_on_top();
        setCentralWidget(editor);
        setWindowTitle(QString((std::string("Gist Console: ") + name).c_str()));

        setAttribute(Qt::WA_QuitOnClose, false);
        setAttribute(Qt::WA_DeleteOnClose, false);
        resize(600, 300);
    }

    TextOutputI::~TextOutputI() {
        delete os;
    }

    std::ostream&
    TextOutputI::get_stream() {
        return *os;
    }

    void
    TextOutputI::flush() {
        static_cast<GistOutputStream*>(os)->flush();
    }

    void
    TextOutputI::insert_html(const QString& s) {
        QTextBlockFormat bf = editor->textCursor().blockFormat();
        bf.setBottomMargin(0);
        editor->textCursor().setBlockFormat(bf);
        editor->insertHtml(s);
        editor->ensureCursorVisible();
    }

    void
    TextOutputI::change_stay_on_top() {
        QPoint p = pos();
        if (stay_on_top->isChecked()) {
            setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        } else {
            setWindowFlags(windowFlags() & ~Qt::WindowStaysOnTopHint);
        }
        move(p);
        show();
    }

} // namespace Gecode::Gist

// STATISTICS: gist-any
