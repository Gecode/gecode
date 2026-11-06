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

#include <gecode/gist/drawingcursor.hh>

namespace Gecode::Gist {

    /// Red color for failed nodes
    const QColor DrawingCursor::red(218, 37, 29);
    /// Green color for solved nodes
    const QColor DrawingCursor::green(11, 118, 70);
    /// Blue color for choice nodes
    const QColor DrawingCursor::blue(0, 92, 161);
    /// Orange color for best solutions
    const QColor DrawingCursor::orange(235, 137, 27);
    /// White color
    const QColor DrawingCursor::white(255, 255, 255);

    /// Red color for expanded failed nodes
    const QColor DrawingCursor::light_red(218, 37, 29, 120);
    /// Green color for expanded solved nodes
    const QColor DrawingCursor::light_green(11, 118, 70, 120);
    /// Blue color for expanded choice nodes
    const QColor DrawingCursor::light_blue(0, 92, 161, 120);

    const double node_width = 20.0;
    const double half_node_width = node_width / 2.0;
    const double failed_width = 14.0;
    const double half_failed_width = failed_width / 2.0;
    const double quarter_failed_width_f = failed_width / 4.0;
    const double shadow_offset = 3.0;
    const double hidden_depth = static_cast<double>(Layout::dist_y) + failed_width;

    DrawingCursor::DrawingCursor(VisualNode* root, const VisualNode::NodeAllocator& na, BestNode* cur_best0,
                                 QPainter& painter0, const QRect& clipping_rect0, bool show_copies)
        : NodeCursor<VisualNode>(root, na), painter(painter0), clipping_rect(clipping_rect0), cur_best(cur_best0),
          x(0.0), y(0.0), copies(show_copies) {
        QPen pen = painter.pen();
        pen.setWidth(1);
        painter.setPen(pen);
    }

    void
    DrawingCursor::process_current_node() {
        Gist::VisualNode* n = node();
        double parent_x = x - static_cast<double>(n->get_offset());
        double parent_y = y - static_cast<double>(Layout::dist_y) + node_width;
        if (!n->is_root() && (n->get_parent(na)->get_status() == STOP || n->get_parent(na)->get_status() == UNSTOP))
            parent_y -= (node_width - failed_width) / 2;

        double myx = x;
        double myy = y;

        if (n->get_status() == STOP || n->get_status() == UNSTOP)
            myy += (node_width - failed_width) / 2;

        if (n != start_node()) {
            if (n->is_on_path())
                painter.setPen(Qt::red);
            else
                painter.setPen(Qt::black);
            // Here we use drawPath instead of drawLine in order to
            // workaround a strange redraw artefact on Windows
            QPainterPath path;
            path.moveTo(myx, myy);
            path.lineTo(parent_x, parent_y);
            painter.drawPath(path);

            QFontMetrics fm = painter.fontMetrics();
            QString label = na.get_label(n);
            int alt = n->get_alternative(na);
            int n_alt = n->get_parent(na)->get_number_of_children();
#if QT_VERSION >= 0x060000
            int tw = fm.horizontalAdvance(label);
#else
            int tw = fm.width(label);
#endif
            int lx;
            if (alt == 0 && n_alt > 1) {
                lx = myx - tw - 4;
            } else if (alt == n_alt - 1 && n_alt > 1) {
                lx = myx + 4;
            } else {
                const int half_text_width = tw / 2;
                lx = myx - half_text_width;
            }
            painter.drawText(QPointF(lx, myy - 2), label);
        }

        // draw shadow
        if (n->is_marked()) {
            painter.setBrush(Qt::gray);
            painter.setPen(Qt::NoPen);
            if (n->is_hidden()) {
                QPointF points[3] = {
                    QPointF(myx + shadow_offset, myy + shadow_offset),
                    QPointF(myx + node_width + shadow_offset, myy + hidden_depth + shadow_offset),
                    QPointF(myx - node_width + shadow_offset, myy + hidden_depth + shadow_offset),
                };
                painter.drawConvexPolygon(points, 3);

            } else {
                switch (n->get_status()) {
                case Gist::SOLVED: {
                    QPointF points[4] = {
                        QPointF(myx + shadow_offset, myy + shadow_offset),
                        QPointF(myx + half_node_width + shadow_offset, myy + half_node_width + shadow_offset),
                        QPointF(myx + shadow_offset, myy + node_width + shadow_offset),
                        QPointF(myx - half_node_width + shadow_offset, myy + half_node_width + shadow_offset)};
                    painter.drawConvexPolygon(points, 4);
                } break;
                case Gist::FAILED:
                    painter.drawRect(myx - half_failed_width + shadow_offset, myy + shadow_offset, failed_width,
                                     failed_width);
                    break;
                case Gist::UNSTOP:
                case Gist::STOP: {
                    QPointF points[8] = {
                        QPointF(myx + shadow_offset - quarter_failed_width_f, myy + shadow_offset),
                        QPointF(myx + shadow_offset + quarter_failed_width_f, myy + shadow_offset),
                        QPointF(myx + shadow_offset + half_failed_width, myy + shadow_offset + quarter_failed_width_f),
                        QPointF(myx + shadow_offset + half_failed_width,
                                myy + shadow_offset + half_failed_width + quarter_failed_width_f),
                        QPointF(myx + shadow_offset + quarter_failed_width_f, myy + shadow_offset + failed_width),
                        QPointF(myx + shadow_offset - quarter_failed_width_f, myy + shadow_offset + failed_width),
                        QPointF(myx + shadow_offset - half_failed_width,
                                myy + shadow_offset + half_failed_width + quarter_failed_width_f),
                        QPointF(myx + shadow_offset - half_failed_width, myy + shadow_offset + quarter_failed_width_f),
                    };
                    painter.drawConvexPolygon(points, 8);
                } break;
                case Gist::BRANCH:
                    painter.drawEllipse(myx - half_node_width + shadow_offset, myy + shadow_offset, node_width,
                                        node_width);
                    break;
                case Gist::UNDETERMINED:
                    painter.drawEllipse(myx - half_node_width + shadow_offset, myy + shadow_offset, node_width,
                                        node_width);
                    break;
                }
            }
        }

        painter.setPen(Qt::SolidLine);
        if (n->is_hidden()) {
            if (n->has_open_children()) {
                QLinearGradient gradient(myx - node_width, myy, myx + node_width * 1.3, myy + hidden_depth * 1.3);
                if (n->has_solved_children()) {
                    gradient.setColorAt(0, white);
                    gradient.setColorAt(1, green);
                } else if (n->has_failed_children()) {
                    gradient.setColorAt(0, white);
                    gradient.setColorAt(1, red);
                } else {
                    gradient.setColorAt(0, white);
                    gradient.setColorAt(1, QColor(0, 0, 0));
                }
                painter.setBrush(gradient);
            } else {
                if (n->has_solved_children())
                    painter.setBrush(QBrush(green));
                else
                    painter.setBrush(QBrush(red));
            }

            QPointF points[3] = {
                QPointF(myx, myy),
                QPointF(myx + node_width, myy + hidden_depth),
                QPointF(myx - node_width, myy + hidden_depth),
            };
            painter.drawConvexPolygon(points, 3);
        } else {
            switch (n->get_status()) {
            case Gist::SOLVED: {
                if (n->is_current_best(cur_best)) {
                    painter.setBrush(QBrush(orange));
                } else {
                    painter.setBrush(QBrush(green));
                }
                QPointF points[4] = {QPointF(myx, myy), QPointF(myx + half_node_width, myy + half_node_width),
                                     QPointF(myx, myy + node_width),
                                     QPointF(myx - half_node_width, myy + half_node_width)};
                painter.drawConvexPolygon(points, 4);
            } break;
            case Gist::FAILED:
                painter.setBrush(QBrush(red));
                painter.drawRect(myx - half_failed_width, myy, failed_width, failed_width);
                break;
            case Gist::UNSTOP:
            case Gist::STOP: {
                painter.setBrush(n->get_status() == STOP ? QBrush(red) : QBrush(green));
                QPointF points[8] = {
                    QPointF(myx - quarter_failed_width_f, myy),
                    QPointF(myx + quarter_failed_width_f, myy),
                    QPointF(myx + half_failed_width, myy + quarter_failed_width_f),
                    QPointF(myx + half_failed_width, myy + half_failed_width + quarter_failed_width_f),
                    QPointF(myx + quarter_failed_width_f, myy + failed_width),
                    QPointF(myx - quarter_failed_width_f, myy + failed_width),
                    QPointF(myx - half_failed_width, myy + half_failed_width + quarter_failed_width_f),
                    QPointF(myx - half_failed_width, myy + quarter_failed_width_f),
                };
                painter.drawConvexPolygon(points, 8);
            } break;
            case Gist::BRANCH:
                painter.setBrush(n->children_layout_is_done() ? QBrush(blue) : QBrush(white));
                painter.drawEllipse(myx - half_node_width, myy, node_width, node_width);
                break;
            case Gist::UNDETERMINED:
                painter.setBrush(Qt::white);
                painter.drawEllipse(myx - half_node_width, myy, node_width, node_width);
                break;
            }
        }

        if (copies && (n->has_copy() && !n->has_working_space())) {
            painter.setBrush(Qt::darkRed);
            painter.drawEllipse(myx, myy, 10.0, 10.0);
        }

        if (copies && n->has_working_space()) {
            painter.setBrush(Qt::darkYellow);
            painter.drawEllipse(myx, myy + 10.0, 10.0, 10.0);
        }

        if (n->is_bookmarked()) {
            painter.setBrush(Qt::black);
            painter.drawEllipse(myx - 10 - 0, myy, 10.0, 10.0);
        }
    }

} // namespace Gecode::Gist

// STATISTICS: gist-any
