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

#include <QApplication>
#include <QMainWindow>

#include <gecode/int.hh>
#include <gecode/gist/qtgist.hh>
#include <gecode/gist/mainwindow.hh>

using namespace Gecode;

/// Space for testing
class TestSpace : public Space {
public:
    IntVarArray variables;
    TestSpace() : variables(*this, 2, 0, 2) {
        branch(*this, variables, INT_VAR_NONE(), INT_VAL_MIN());
    }
    TestSpace(TestSpace& other) : Space(other) {
        variables.update(*this, other.variables);
    }
    Space*
    copy() override {
        return new TestSpace(*this);
    }
    void
    print(std::ostream& os) const {
        os << variables << '\n';
    }
};

int
main(int argc, char** argv) {
    QApplication app(argc, argv);
    Gist::Print<TestSpace> printer("My Gist");
    Gist::Options options;
    options.inspect.click(&printer);

    Gist::GistMainWindow gist_window(new TestSpace(), false, options);
    gist_window.show();

    QMainWindow widget_window;
    Gist::Gist gist_widget(new TestSpace(), false, &widget_window, options);
    widget_window.setCentralWidget(&gist_widget);
    widget_window.resize(500, 300);
    widget_window.show();

    return QApplication::exec();
}
