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

#include <gecode/gist/nodestats.hh>
#include <gecode/gist/nodewidget.hh>
#include <gecode/gist/nodecursor.hh>
#include <gecode/gist/nodevisitor.hh>
#include <gecode/gist/drawingcursor.hh>

namespace Gecode { namespace Gist {

    NodeStatInspector::NodeStatInspector(QWidget* parent) : QWidget(parent) {
        setWindowFlags(Qt::Tool);
        QGraphicsScene* scene = new QGraphicsScene();

        scene->addEllipse(70, 10, 16, 16, QPen(), QBrush(DrawingCursor::white));
        scene->addEllipse(70, 60, 16, 16, QPen(), QBrush(DrawingCursor::blue));
        scene->addRect(32, 100, 12, 12, QPen(), QBrush(DrawingCursor::red));

        QPolygonF poly;
        poly << QPointF(78, 100) << QPointF(78 + 8, 100 + 8) << QPointF(78, 100 + 16) << QPointF(78 - 8, 100 + 8);
        scene->addPolygon(poly, QPen(), QBrush(DrawingCursor::green));

        scene->addEllipse(110, 100, 16, 16, QPen(), QBrush(DrawingCursor::white));

        QPen pen;
        pen.setStyle(Qt::DotLine);
        pen.setWidth(0);
        scene->addLine(78, 26, 78, 60, pen);
        scene->addLine(78, 76, 38, 100, pen);
        scene->addLine(78, 76, 78, 100, pen);
        scene->addLine(78, 76, 118, 100, pen);

        scene->addLine(135, 10, 145, 10);
        scene->addLine(145, 10, 145, 110);
        scene->addLine(145, 60, 135, 60);
        scene->addLine(145, 110, 135, 110);

        node_depth_label = scene->addText("0");
        node_depth_label->setPos(150, 20);
        subtree_depth_label = scene->addText("0");
        subtree_depth_label->setPos(150, 75);

        choices_label = scene->addText("0");
        choices_label->setPos(45, 57);

        solved_label = scene->addText("0");
        solved_label->setPos(78 - solved_label->document()->size().width() / 2, 120);
        failed_label = scene->addText("0");
        failed_label->setPos(30, 120);
        open_label = scene->addText("0");
        open_label->setPos(110, 120);

        QGraphicsView* view = new QGraphicsView(scene);
        view->setRenderHints(view->renderHints() | QPainter::Antialiasing);
        view->show();

        scene->setBackgroundBrush(Qt::white);

        box_layout = new QVBoxLayout();
        box_layout->setContentsMargins(0, 0, 0, 0);
        box_layout->addWidget(view);
        setLayout(box_layout);

        setWindowTitle("Gist node statistics");
        setAttribute(Qt::WA_QuitOnClose, false);
        setAttribute(Qt::WA_DeleteOnClose, false);
    }

    void
    NodeStatInspector::node(const VisualNode::NodeAllocator& na, VisualNode* n, const Statistics&, bool) {
        if (isVisible()) {
            int nd = -1;
            for (VisualNode* p = n; p != nullptr; p = p->get_parent(na))
                nd++;
            node_depth_label->setPlainText(QString("%1").arg(nd));
            ;
            StatCursor sc(n, na);
            PreorderNodeVisitor<StatCursor> pnv(sc);
            pnv.run();

            subtree_depth_label->setPlainText(QString("%1").arg(pnv.get_cursor().depth));
            solved_label->setPlainText(QString("%1").arg(pnv.get_cursor().solved));
            solved_label->setPos(78 - solved_label->document()->size().width() / 2, 120);
            failed_label->setPlainText(QString("%1").arg(pnv.get_cursor().failed));
            failed_label->setPos(44 - failed_label->document()->size().width(), 120);
            choices_label->setPlainText(QString("%1").arg(pnv.get_cursor().choice));
            choices_label->setPos(66 - choices_label->document()->size().width(), 57);
            open_label->setPlainText(QString("%1").arg(pnv.get_cursor().open));
        }
    }

    void
    NodeStatInspector::show_stats() {
        show();
        activateWindow();
    }

}} // namespace Gecode::Gist

// STATISTICS: gist-any
