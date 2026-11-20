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

#include <QtGui/QPainter>
#include <QPrinter>
#include <QPrintDialog>

#include <stack>
#include <fstream>

#include <gecode/gist/treecanvas.hh>

#include <gecode/gist/nodevisitor.hh>
#include <gecode/gist/visualnode.hh>
#include <gecode/gist/drawingcursor.hh>

#include <gecode/search.hh>
#include <gecode/search/support.hh>

namespace Gecode { namespace Gist {

    TreeCanvas::TreeCanvas(Space* root_space, bool bab, QWidget* parent, const Options& opt)
        : QWidget(parent)
#if QT_VERSION < 0x060000
          ,
          mutex(QMutex::Recursive), layout_mutex(QMutex::Recursive)
#endif
          ,
          finished_flag(false), compare_nodes(false), compare_nodes_before_fp(false), auto_hide_failed(true),
          auto_zoom(false), refresh(500), refresh_pause(0), smooth_scroll_and_zoom(false), move_during_search(false),
          zoom_time_line(500), scroll_time_line(1000), target_x(0), source_x(0), target_y(0), source_y(0), target_w(0),
          target_h(0), target_scale(0), layout_done_timer_id(0) {
        QMutexLocker locker(&mutex);
        cur_best = (bab ? new BestNode(nullptr) : nullptr);
        if (root_space->status() == SS_FAILED) {
            if (!opt.clone)
                delete root_space;
            root_space = nullptr;
        } else {
            root_space = Gecode::Search::snapshot(root_space, opt);
        }
        na = new Node::NodeAllocator(bab);
        int root_idx = na->allocate(root_space);
        assert(root_idx == 0);
        (void)root_idx;
        root = (*na)[0];
        root->layout(*na);
        root->set_marked(true);
        current_node = root;
        path_head = root;
        scale = LayoutConfig::def_scale / 100.0;

        setAutoFillBackground(true);

        connect(&searcher, SIGNAL(update(int, int, int)), this, SLOT(layout_done(int, int, int)));
        connect(&searcher, SIGNAL(status_changed(bool)), this, SLOT(status_changed(bool)));

        connect(&searcher, SIGNAL(solution(const Space*)), this, SIGNAL(solution(const Space*)),
                Qt::BlockingQueuedConnection);
        connect(this, SIGNAL(solution(const Space*)), this, SLOT(inspect_solution(const Space*)));

        connect(&searcher, SIGNAL(move_to_node(VisualNode*, bool)), this, SLOT(set_current_node(VisualNode*, bool)),
                Qt::BlockingQueuedConnection);

        connect(&searcher, SIGNAL(search_finished(void)), this, SIGNAL(search_finished(void)));

        connect(&scroll_time_line, SIGNAL(frameChanged(int)), this, SLOT(scroll(int)));
#if QT_VERSION >= 0x060000
        scroll_time_line.setEasingCurve(QEasingCurve::InOutSine);
#else
        scroll_time_line.setCurveShape(QTimeLine::EaseInOutCurve);
#endif

        scale_bar = new QSlider(Qt::Vertical, this);
        scale_bar->setObjectName("scaleBar");
        scale_bar->setMinimum(LayoutConfig::min_scale);
        scale_bar->setMaximum(LayoutConfig::max_scale);
        scale_bar->setValue(LayoutConfig::def_scale);
        connect(scale_bar, SIGNAL(valueChanged(int)), this, SLOT(scale_tree(int)));
        connect(this, SIGNAL(scale_changed(int)), scale_bar, SLOT(setValue(int)));
        connect(&searcher, SIGNAL(scale_changed(int)), scale_bar, SLOT(setValue(int)));

        connect(&zoom_time_line, SIGNAL(frameChanged(int)), scale_bar, SLOT(setValue(int)));
#if QT_VERSION >= 0x060000
        zoom_time_line.setEasingCurve(QEasingCurve::InOutSine);
#else
        zoom_time_line.setCurveShape(QTimeLine::EaseInOutCurve);
#endif

        qRegisterMetaType<Statistics>("Statistics");
        update();
    }

    TreeCanvas::~TreeCanvas(void) {
        if (root) {
            DisposeCursor dc(root, *na);
            PreorderNodeVisitor<DisposeCursor>(dc).run();
        }
        delete na;
    }

    void
    TreeCanvas::add_double_click_inspector(Inspector* i) {
        double_click_inspectors.append(QPair<Inspector*, bool>(i, false));
    }

    void
    TreeCanvas::activate_double_click_inspector(int i, bool active) {
        assert(i < double_click_inspectors.size());
        double_click_inspectors[i].second = active;
    }

    void
    TreeCanvas::add_solution_inspector(Inspector* i) {
        solution_inspectors.append(QPair<Inspector*, bool>(i, false));
    }

    void
    TreeCanvas::activate_solution_inspector(int i, bool active) {
        assert(i < solution_inspectors.size());
        solution_inspectors[i].second = active;
    }

    void
    TreeCanvas::add_move_inspector(Inspector* i) {
        move_inspectors.append(QPair<Inspector*, bool>(i, false));
    }

    void
    TreeCanvas::activate_move_inspector(int i, bool active) {
        assert(i < move_inspectors.size());
        move_inspectors[i].second = active;
    }

    void
    TreeCanvas::add_comparator(Comparator* c) {
        comparators.append(QPair<Comparator*, bool>(c, false));
    }

    void
    TreeCanvas::activate_comparator(int i, bool active) {
        assert(i < comparators.size());
        comparators[i].second = active;
    }

    void
    TreeCanvas::scale_tree(int scale0, int zoomx, int zoomy) {
        QMutexLocker locker(&layout_mutex);

        QSize viewport_size = size();
        QAbstractScrollArea* sa = static_cast<QAbstractScrollArea*>(parentWidget()->parentWidget());

        if (zoomx == -1)
            zoomx = viewport_size.width() / 2;
        if (zoomy == -1)
            zoomy = viewport_size.height() / 2;

        int xoff = (sa->horizontalScrollBar()->value() + zoomx) / scale;
        int yoff = (sa->verticalScrollBar()->value() + zoomy) / scale;

        BoundingBox bb;
        scale0 = std::min(std::max(scale0, LayoutConfig::min_scale), LayoutConfig::max_scale);
        scale = (static_cast<double>(scale0)) / 100.0;
        bb = root->get_bounding_box();
        int w = static_cast<int>((bb.right - bb.left + Layout::extent) * scale);
        int h = static_cast<int>(2 * Layout::extent + root->get_shape()->depth() * Layout::dist_y * scale);

        sa->horizontalScrollBar()->setRange(0, w - viewport_size.width());
        sa->verticalScrollBar()->setRange(0, h - viewport_size.height());
        sa->horizontalScrollBar()->setPageStep(viewport_size.width());
        sa->verticalScrollBar()->setPageStep(viewport_size.height());
        sa->horizontalScrollBar()->setSingleStep(Layout::extent);
        sa->verticalScrollBar()->setSingleStep(Layout::extent);

        xoff *= scale;
        yoff *= scale;

        sa->horizontalScrollBar()->setValue(xoff - zoomx);
        sa->verticalScrollBar()->setValue(yoff - zoomy);

        emit scale_changed(scale0);
        QWidget::update();
    }

    void
    TreeCanvas::update(void) {
        QMutexLocker locker(&mutex);
        layout_mutex.lock();
        if (root != nullptr) {
            root->layout(*na);
            BoundingBox bb = root->get_bounding_box();

            int w = static_cast<int>((bb.right - bb.left + Layout::extent) * scale);
            int h = static_cast<int>(2 * Layout::extent + root->get_shape()->depth() * Layout::dist_y * scale);
            xtrans = -bb.left + (Layout::extent / 2);

            QSize viewport_size = size();
            QAbstractScrollArea* sa = static_cast<QAbstractScrollArea*>(parentWidget()->parentWidget());
            sa->horizontalScrollBar()->setRange(0, w - viewport_size.width());
            sa->verticalScrollBar()->setRange(0, h - viewport_size.height());
            sa->horizontalScrollBar()->setPageStep(viewport_size.width());
            sa->verticalScrollBar()->setPageStep(viewport_size.height());
            sa->horizontalScrollBar()->setSingleStep(Layout::extent);
            sa->verticalScrollBar()->setSingleStep(Layout::extent);
        }
        if (auto_zoom)
            zoom_to_fit();
        layout_mutex.unlock();
        QWidget::update();
    }

    void
    TreeCanvas::scroll(void) {
        QWidget::update();
    }

    void
    TreeCanvas::layout_done(int w, int h, int scale0) {
        target_w = w;
        target_h = h;
        target_scale = scale0;

        QSize viewport_size = size();
        QAbstractScrollArea* sa = static_cast<QAbstractScrollArea*>(parentWidget()->parentWidget());
        sa->horizontalScrollBar()->setRange(0, w - viewport_size.width());
        sa->verticalScrollBar()->setRange(0, h - viewport_size.height());

        if (layout_done_timer_id == 0)
            layout_done_timer_id = startTimer(15);
    }

    void
    TreeCanvas::status_changed(bool finished) {
        if (finished) {
            update();
            center_current_node();
        }
        emit status_changed(current_node, stats, finished);
    }

    void
    SearcherThread::search(VisualNode* n, bool all, TreeCanvas* ti) {
        node = n;

        depth = -1;
        for (VisualNode* p = n; p != nullptr; p = p->get_parent(*ti->na))
            depth++;

        a = all;
        t = ti;
        start();
    }

    void
    SearcherThread::update_canvas(void) {
        t->layout_mutex.lock();
        if (t->root == nullptr)
            return;

        if (t->auto_hide_failed) {
            t->root->hide_failed(*t->na, true);
        }
        for (VisualNode* n = t->current_node; n != nullptr; n = n->get_parent(*t->na)) {
            if (n->is_hidden()) {
                t->current_node->set_marked(false);
                t->current_node = n;
                t->current_node->set_marked(true);
                break;
            }
        }

        t->root->layout(*t->na);
        BoundingBox bb = t->root->get_bounding_box();

        int w = static_cast<int>((bb.right - bb.left + Layout::extent) * t->scale);
        int h = static_cast<int>(2 * Layout::extent + t->root->get_shape()->depth() * Layout::dist_y * t->scale);
        t->xtrans = -bb.left + (Layout::extent / 2);

        int scale0 = static_cast<int>(t->scale * 100);
        if (t->auto_zoom) {
            QWidget* p = t->parentWidget();
            if (p) {
                double new_x_scale = static_cast<double>(p->width()) / (bb.right - bb.left + Layout::extent);
                double new_y_scale = static_cast<double>(p->height()) /
                                     (t->root->get_shape()->depth() * Layout::dist_y + 2 * Layout::extent);

                scale0 = static_cast<int>(std::min(new_x_scale, new_y_scale) * 100);
                if (scale0 < LayoutConfig::min_scale)
                    scale0 = LayoutConfig::min_scale;
                if (scale0 > LayoutConfig::max_auto_zoom_scale)
                    scale0 = LayoutConfig::max_auto_zoom_scale;
                double scale = (static_cast<double>(scale0)) / 100.0;

                w = static_cast<int>((bb.right - bb.left + Layout::extent) * scale);
                h = static_cast<int>(2 * Layout::extent + t->root->get_shape()->depth() * Layout::dist_y * scale);
            }
        }

        t->layout_mutex.unlock();
        emit update(w, h, scale0);
    }

    /// A stack item for depth first search
    class SearchItem {
    public:
        /// The node
        VisualNode* n;
        /// The currently explored child
        int i;
        /// The number of children
        int no_of_children;
        /// Constructor
        SearchItem(VisualNode* n0, int no_of_children0) : n(n0), i(-1), no_of_children(no_of_children0) {}
    };

    void
    SearcherThread::run(void) {
        {
            if (!node->is_open())
                return;
            t->mutex.lock();
            emit status_changed(false);

            unsigned int kids = node->get_number_of_child_nodes(*t->na, t->cur_best, t->stats, t->c_d, t->a_d);
            if (kids == 0 || node->get_status() == STOP) {
                t->mutex.unlock();
                update_canvas();
                emit status_changed(true);
                return;
            }

            std::stack<SearchItem> stck;
            stck.push(SearchItem(node, kids));
            t->stats.max_depth = std::max(static_cast<long unsigned int>(t->stats.max_depth),
                                          static_cast<long unsigned int>(depth + stck.size()));

            VisualNode* sol = nullptr;
            int node_count = 0;
            t->stop_search_flag = false;
            while (!stck.empty() && !t->stop_search_flag) {
                if (t->refresh > 0 && node_count >= t->refresh) {
                    node->dirty_up(*t->na);
                    update_canvas();
                    emit status_changed(false);
                    node_count = 0;
                    if (t->refresh_pause > 0)
                        msleep(t->refresh_pause);
                }
                SearchItem& si = stck.top();
                si.i++;
                if (si.i == si.no_of_children) {
                    stck.pop();
                } else {
                    VisualNode* n = si.n->get_child(*t->na, si.i);
                    if (n->is_open()) {
                        if (n->get_status() == UNDETERMINED)
                            node_count++;
                        kids = n->get_number_of_child_nodes(*t->na, t->cur_best, t->stats, t->c_d, t->a_d);
                        if (t->move_during_search)
                            emit move_to_node(n, false);
                        if (kids == 0) {
                            if (n->get_status() == SOLVED) {
                                assert(n->has_copy());
                                emit solution(n->get_working_space());
                                n->purge(*t->na);
                                sol = n;
                                if (!a)
                                    break;
                            }
                        } else {
                            if (n->get_status() != STOP)
                                stck.push(SearchItem(n, kids));
                            else if (!a)
                                break;
                            t->stats.max_depth = std::max(static_cast<long unsigned int>(t->stats.max_depth),
                                                          static_cast<long unsigned int>(depth + stck.size()));
                        }
                    }
                }
            }
            node->dirty_up(*t->na);
            t->stop_search_flag = false;
            t->mutex.unlock();
            if (sol != nullptr) {
                t->set_current_node(sol, true, false);
            } else {
                t->set_current_node(node, true, false);
            }
        }
        update_canvas();
        emit status_changed(true);
        if (t->finished_flag)
            emit search_finished();
    }

    void
    TreeCanvas::search_all(void) {
        QMutexLocker locker(&mutex);
        searcher.search(current_node, true, this);
    }

    void
    TreeCanvas::search_one(void) {
        QMutexLocker locker(&mutex);
        searcher.search(current_node, false, this);
    }

    void
    TreeCanvas::toggle_hidden(void) {
        QMutexLocker locker(&mutex);
        current_node->toggle_hidden(*na);
        update();
        center_current_node();
        emit status_changed(current_node, stats, true);
    }

    void
    TreeCanvas::hide_failed(void) {
        QMutexLocker locker(&mutex);
        current_node->hide_failed(*na);
        update();
        center_current_node();
        emit status_changed(current_node, stats, true);
    }

    void
    TreeCanvas::unhide_all(void) {
        QMutexLocker locker(&mutex);
        QMutexLocker layout_locker(&layout_mutex);
        current_node->unhide_all(*na);
        update();
        center_current_node();
        emit status_changed(current_node, stats, true);
    }

    void
    TreeCanvas::toggle_stop(void) {
        QMutexLocker locker(&mutex);
        current_node->toggle_stop(*na);
        update();
        center_current_node();
        emit status_changed(current_node, stats, true);
    }

    void
    TreeCanvas::unstop_all(void) {
        QMutexLocker locker(&mutex);
        QMutexLocker layout_locker(&layout_mutex);
        current_node->unstop_all(*na);
        update();
        center_current_node();
        emit status_changed(current_node, stats, true);
    }

    void
    TreeCanvas::timerEvent(QTimerEvent* e) {
        if (e->timerId() == layout_done_timer_id) {
            if (!smooth_scroll_and_zoom) {
                scale_tree(target_scale);
            } else {
                zoom_time_line.stop();
                int zoom_current = static_cast<int>(scale * 100);
                int target_zoom = target_scale;
                target_zoom =
                    std::min(std::max(target_zoom, LayoutConfig::min_scale), LayoutConfig::max_auto_zoom_scale);
                zoom_time_line.setFrameRange(zoom_current, target_zoom);
                zoom_time_line.start();
            }
            QWidget::update();
            killTimer(layout_done_timer_id);
            layout_done_timer_id = 0;
        }
    }

    void
    TreeCanvas::zoom_to_fit(void) {
        QMutexLocker locker(&layout_mutex);
        if (root != nullptr) {
            BoundingBox bb;
            bb = root->get_bounding_box();
            QWidget* p = parentWidget();
            if (p) {
                double new_x_scale = static_cast<double>(p->width()) / (bb.right - bb.left + Layout::extent);
                double new_y_scale = static_cast<double>(p->height()) /
                                     (root->get_shape()->depth() * Layout::dist_y + 2 * Layout::extent);
                int scale0 = static_cast<int>(std::min(new_x_scale, new_y_scale) * 100);
                if (scale0 < LayoutConfig::min_scale)
                    scale0 = LayoutConfig::min_scale;
                if (scale0 > LayoutConfig::max_auto_zoom_scale)
                    scale0 = LayoutConfig::max_auto_zoom_scale;

                if (!smooth_scroll_and_zoom) {
                    scale_tree(scale0);
                } else {
                    zoom_time_line.stop();
                    int zoom_current = static_cast<int>(scale * 100);
                    int target_zoom = scale0;
                    target_zoom =
                        std::min(std::max(target_zoom, LayoutConfig::min_scale), LayoutConfig::max_auto_zoom_scale);
                    zoom_time_line.setFrameRange(zoom_current, target_zoom);
                    zoom_time_line.start();
                }
            }
        }
    }

    void
    TreeCanvas::center_current_node(void) {
        QMutexLocker locker(&mutex);
        int x = 0;
        int y = 0;

        VisualNode* c = current_node;
        while (c != nullptr) {
            x += c->get_offset();
            y += Layout::dist_y;
            c = c->get_parent(*na);
        }

        x = static_cast<int>((xtrans + x) * scale);
        y = static_cast<int>(y * scale);

        QAbstractScrollArea* sa = static_cast<QAbstractScrollArea*>(parentWidget()->parentWidget());

        x -= sa->viewport()->width() / 2;
        y -= sa->viewport()->height() / 2;

        source_x = sa->horizontalScrollBar()->value();
        target_x = std::max(sa->horizontalScrollBar()->minimum(), x);
        target_x = std::min(sa->horizontalScrollBar()->maximum(), target_x);
        source_y = sa->verticalScrollBar()->value();
        target_y = std::max(sa->verticalScrollBar()->minimum(), y);
        target_y = std::min(sa->verticalScrollBar()->maximum(), target_y);
        if (!smooth_scroll_and_zoom) {
            sa->horizontalScrollBar()->setValue(target_x);
            sa->verticalScrollBar()->setValue(target_y);
        } else {
            scroll_time_line.stop();
            scroll_time_line.setFrameRange(0, 100);
            scroll_time_line.setDuration(
                std::max(200, std::min(1000, std::min(std::abs(source_x - target_x), std::abs(source_y - target_y)))));
            scroll_time_line.start();
        }
    }

    void
    TreeCanvas::scroll(int i) {
        QAbstractScrollArea* sa = static_cast<QAbstractScrollArea*>(parentWidget()->parentWidget());
        double p = static_cast<double>(i) / 100.0;
        double xdiff = static_cast<double>(target_x - source_x) * p;
        double ydiff = static_cast<double>(target_y - source_y) * p;
        sa->horizontalScrollBar()->setValue(source_x + static_cast<int>(xdiff));
        sa->verticalScrollBar()->setValue(source_y + static_cast<int>(ydiff));
    }

    void
    TreeCanvas::inspect_current_node(bool fix, int inspector_no) {
        QMutexLocker locker(&mutex);

        if (current_node->is_hidden()) {
            toggle_hidden();
            return;
        }

        int failed_inspector_type = -1;
        int failed_inspector = -1;
        bool need_centering = false;
        try {
            switch (current_node->get_status()) {
            case UNDETERMINED: {
                unsigned int kids = current_node->get_number_of_child_nodes(*na, cur_best, stats, c_d, a_d);
                int depth = -1;
                for (VisualNode* p = current_node; p != nullptr; p = p->get_parent(*na))
                    depth++;
                if (kids > 0) {
                    need_centering = true;
                    depth++;
                }
                stats.max_depth = std::max(stats.max_depth, depth);
                if (current_node->get_status() == SOLVED) {
                    assert(current_node->has_copy());
                    emit solution(current_node->get_working_space());
                }
                emit status_changed(current_node, stats, true);
                for (int i = 0; i < move_inspectors.size(); i++) {
                    if (move_inspectors[i].second) {
                        failed_inspector_type = 0;
                        failed_inspector = i;
                        if (current_node->get_status() == FAILED) {
                            if (!current_node->is_root()) {
                                Space* cur_space = current_node->get_space(*na, cur_best, c_d, a_d);
                                move_inspectors[i].first->inspect(*cur_space);
                                delete cur_space;
                            }
                        } else {
                            move_inspectors[i].first->inspect(*current_node->get_working_space());
                        }
                        failed_inspector_type = -1;
                    }
                }
                if (current_node->get_status() == SOLVED) {
                    current_node->purge(*na);
                }
            } break;
            case FAILED:
            case STOP:
            case UNSTOP:
            case BRANCH:
            case SOLVED: {
                Space* cur_space;

                if (fix) {
                    if (current_node->is_root() && current_node->get_status() == FAILED)
                        break;
                    cur_space = current_node->get_space(*na, cur_best, c_d, a_d);
                    if (current_node->get_status() == SOLVED && cur_space->status() != SS_SOLVED) {
                        // in the presence of weakly monotonic propagators, we may have
                        // to use search to find the solution here
                        assert(cur_space->status() == SS_BRANCH &&
                               "Something went wrong - probably an incorrect brancher");
                        Space* dfs_space = Gecode::dfs(cur_space);
                        delete cur_space;
                        cur_space = dfs_space;
                    }
                } else {
                    if (current_node->is_root())
                        break;
                    VisualNode* p = current_node->get_parent(*na);
                    cur_space = p->get_space(*na, cur_best, c_d, a_d);
                    switch (cur_space->status()) {
                    case SS_SOLVED:
                    case SS_FAILED:
                        break;
                    case SS_BRANCH:
                        cur_space->commit(*p->get_choice(), current_node->get_alternative(*na));
                        break;
                    default:
                        GECODE_NEVER;
                    }
                }

                if (inspector_no == -1) {
                    for (int i = 0; i < double_click_inspectors.size(); i++) {
                        if (double_click_inspectors[i].second) {
                            failed_inspector_type = 1;
                            failed_inspector = i;
                            double_click_inspectors[i].first->inspect(*cur_space);
                            failed_inspector_type = -1;
                        }
                    }
                } else {
                    failed_inspector_type = 1;
                    failed_inspector = inspector_no;
                    double_click_inspectors[inspector_no].first->inspect(*cur_space);
                    failed_inspector_type = -1;
                }
                delete cur_space;
            } break;
            }
        } catch (Exception& e) {
            switch (failed_inspector_type) {
            case 0:
                qFatal("Exception in move inspector %d: %s.\n Stopping.", failed_inspector, e.what());
                break;
            case 1:
                qFatal("Exception in double click inspector %d: %s.\n Stopping.", failed_inspector, e.what());
                break;
            default:
                qFatal("Exception: %s.\n Stopping.", e.what());
                break;
            }
        }

        current_node->dirty_up(*na);
        update();
        if (need_centering)
            center_current_node();
    }

    void
    TreeCanvas::inspect_before_fp(void) {
        inspect_current_node(false);
    }

    void
    TreeCanvas::label_branches(void) {
        QMutexLocker locker(&mutex);
        current_node->label_branches(*na, cur_best, c_d, a_d);
        update();
        center_current_node();
        emit status_changed(current_node, stats, true);
    }
    void
    TreeCanvas::label_path(void) {
        QMutexLocker locker(&mutex);
        current_node->label_path(*na, cur_best, c_d, a_d);
        update();
        center_current_node();
        emit status_changed(current_node, stats, true);
    }

    void
    TreeCanvas::inspect_solution(const Space* s) {
        int failed_inspector_type = -1;
        int failed_inspector = -1;
        try {
            Space* c = nullptr;
            for (int i = 0; i < solution_inspectors.size(); i++) {
                if (solution_inspectors[i].second) {
                    if (c == nullptr)
                        c = s->clone();
                    failed_inspector_type = 1;
                    failed_inspector = i;
                    solution_inspectors[i].first->inspect(*c);
                    failed_inspector_type = -1;
                }
            }
            delete c;
        } catch (Exception& e) {
            switch (failed_inspector_type) {
            case 0:
                qFatal("Exception in move inspector %d: %s.\n Stopping.", failed_inspector, e.what());
                break;
            case 1:
                qFatal("Exception in solution inspector %d: %s.\n Stopping.", failed_inspector, e.what());
                break;
            default:
                qFatal("Exception: %s.\n Stopping.", e.what());
                break;
            }
        }
    }

    void
    TreeCanvas::stop_search(void) {
        stop_search_flag = true;
        layout_done_timer_id = startTimer(15);
    }

    void
    TreeCanvas::reset(void) {
        QMutexLocker locker(&mutex);
        Space* root_space = root->get_status() == FAILED ? nullptr : root->get_space(*na, cur_best, c_d, a_d);
        if (cur_best != nullptr) {
            delete cur_best;
            cur_best = new BestNode(nullptr);
        }
        if (root) {
            DisposeCursor dc(root, *na);
            PreorderNodeVisitor<DisposeCursor>(dc).run();
        }
        delete na;
        na = new Node::NodeAllocator(cur_best != nullptr);
        int root_idx = na->allocate(root_space);
        assert(root_idx == 0);
        (void)root_idx;
        root = (*na)[0];
        root->set_marked(true);
        current_node = root;
        path_head = root;
        scale = 1.0;
        stats = Statistics();
        for (int i = bookmarks.size(); i--;)
            emit removed_bookmark(i);
        bookmarks.clear();
        root->layout(*na);

        emit status_changed(current_node, stats, true);
        update();
    }

    void
    TreeCanvas::bookmark_node(void) {
        QMutexLocker locker(&mutex);
        if (!current_node->is_bookmarked()) {
            bool ok;
            QString text = QInputDialog::getText(this, "Add bookmark", "Name:", QLineEdit::Normal, "", &ok);
            if (ok) {
                current_node->set_bookmarked(true);
                bookmarks.append(current_node);
                if (text == "")
                    text = QString("Node ") + QString().setNum(bookmarks.size());
                emit added_bookmark(text);
            }
        } else {
            current_node->set_bookmarked(false);
            int idx = bookmarks.indexOf(current_node);
            bookmarks.remove(idx);
            emit removed_bookmark(idx);
        }
        current_node->dirty_up(*na);
        update();
    }

    void
    TreeCanvas::set_path(void) {
        QMutexLocker locker(&mutex);
        if (current_node == path_head)
            return;

        path_head->un_path_up(*na);
        path_head = current_node;

        current_node->path_up(*na);
        current_node->dirty_up(*na);
        update();
    }

    void
    TreeCanvas::inspect_path(void) {
        QMutexLocker locker(&mutex);
        set_current_node(root);
        if (current_node->is_on_path()) {
            inspect_current_node();
            int next_alt = current_node->get_path_alternative(*na);
            while (next_alt >= 0) {
                set_current_node(current_node->get_child(*na, next_alt));
                inspect_current_node();
                next_alt = current_node->get_path_alternative(*na);
            }
        }
        update();
    }

    void
    TreeCanvas::start_compare_nodes(void) {
        QMutexLocker locker(&mutex);
        compare_nodes = true;
        compare_nodes_before_fp = false;
        setCursor(QCursor(Qt::CrossCursor));
    }

    void
    TreeCanvas::start_compare_nodes_before_fp(void) {
        QMutexLocker locker(&mutex);
        compare_nodes = true;
        compare_nodes_before_fp = true;
        setCursor(QCursor(Qt::CrossCursor));
    }

    void
    TreeCanvas::emit_status_changed(void) {
        emit status_changed(current_node, stats, true);
    }

    void
    TreeCanvas::nav_up(void) {
        QMutexLocker locker(&mutex);

        VisualNode* p = current_node->get_parent(*na);

        set_current_node(p);

        if (p != nullptr) {
            center_current_node();
        }
    }

    void
    TreeCanvas::nav_down(void) {
        QMutexLocker locker(&mutex);
        if (!current_node->is_hidden()) {
            switch (current_node->get_status()) {
            case STOP:
            case UNSTOP:
            case BRANCH: {
                int alt = std::max(0, current_node->get_path_alternative(*na));
                VisualNode* n = current_node->get_child(*na, alt);
                set_current_node(n);
                center_current_node();
                break;
            }
            case SOLVED:
            case FAILED:
            case UNDETERMINED:
                break;
            }
        }
    }

    void
    TreeCanvas::nav_left(void) {
        QMutexLocker locker(&mutex);
        VisualNode* p = current_node->get_parent(*na);
        if (p != nullptr) {
            int alt = current_node->get_alternative(*na);
            if (alt > 0) {
                VisualNode* n = p->get_child(*na, alt - 1);
                set_current_node(n);
                center_current_node();
            }
        }
    }

    void
    TreeCanvas::nav_right(void) {
        QMutexLocker locker(&mutex);
        VisualNode* p = current_node->get_parent(*na);
        if (p != nullptr) {
            unsigned int alt = current_node->get_alternative(*na);
            if (alt + 1 < p->get_number_of_children()) {
                VisualNode* n = p->get_child(*na, alt + 1);
                set_current_node(n);
                center_current_node();
            }
        }
    }

    void
    TreeCanvas::nav_root(void) {
        QMutexLocker locker(&mutex);
        set_current_node(root);
        center_current_node();
    }

    void
    TreeCanvas::nav_next_sol(bool back) {
        QMutexLocker locker(&mutex);
        NextSolCursor nsc(current_node, back, *na);
        PreorderNodeVisitor<NextSolCursor> nsv(nsc);
        nsv.run();
        VisualNode* n = nsv.get_cursor().node();
        if (n != root) {
            set_current_node(n);
            center_current_node();
        }
    }

    void
    TreeCanvas::nav_prev_sol(void) {
        nav_next_sol(true);
    }

    void
    TreeCanvas::export_node_pdf(VisualNode* n) {
#if QT_VERSION >= 0x040400
        QString filename = QFileDialog::getSaveFileName(this, tr("Export tree as pdf"), "", tr("PDF (*.pdf)"));
        if (filename != "") {
            QPrinter printer(QPrinter::ScreenResolution);
            QMutexLocker locker(&mutex);

            BoundingBox bb = n->get_bounding_box();
            printer.setFullPage(true);
#if QT_VERSION >= 0x060000
            printer.setPageSize(QPageSize(
                QSizeF(bb.right - bb.left + Layout::extent, n->get_shape()->depth() * Layout::dist_y + Layout::extent),
                QPageSize::Point));
#else
            printer.setPaperSize(
                QSizeF(bb.right - bb.left + Layout::extent, n->get_shape()->depth() * Layout::dist_y + Layout::extent),
                QPrinter::Point);
#endif
            printer.setOutputFileName(filename);
            QPainter painter(&printer);

            painter.setRenderHint(QPainter::Antialiasing);
#if QT_VERSION >= 0x060000
            QRect page_rect = printer.pageLayout().paintRectPixels(printer.resolution());
#else
            QRect page_rect = printer.pageRect();
#endif
            double new_x_scale = static_cast<double>(page_rect.width()) / (bb.right - bb.left + Layout::extent);
            double new_y_scale =
                static_cast<double>(page_rect.height()) / (n->get_shape()->depth() * Layout::dist_y + Layout::extent);
            double print_scale = std::min(new_x_scale, new_y_scale);
            painter.scale(print_scale, print_scale);

            int printxtrans = -bb.left + (Layout::extent / 2);
            const int print_y_translation = Layout::dist_y / 2;

            painter.translate(printxtrans, print_y_translation);
            QRect clip(0, 0, 0, 0);
            DrawingCursor dc(n, *na, cur_best, painter, clip, show_copies);
            current_node->set_marked(false);
            PreorderNodeVisitor<DrawingCursor>(dc).run();
            current_node->set_marked(true);
        }
#else
        (void)n;
#endif
    }

    void
    TreeCanvas::export_whole_tree_pdf(void) {
#if QT_VERSION >= 0x040400
        export_node_pdf(root);
#endif
    }

    void
    TreeCanvas::export_pdf(void) {
#if QT_VERSION >= 0x040400
        export_node_pdf(current_node);
#endif
    }

    void
    TreeCanvas::print(void) {
        QPrinter printer;
        if (QPrintDialog(&printer, this).exec() == QDialog::Accepted) {
            QMutexLocker locker(&mutex);

            BoundingBox bb = root->get_bounding_box();
#if QT_VERSION >= 0x060000
            QRect page_rect = printer.pageLayout().paintRectPixels(printer.resolution());
#else
            QRect page_rect = printer.pageRect();
#endif
            double new_x_scale = static_cast<double>(page_rect.width()) / (bb.right - bb.left + Layout::extent);
            double new_y_scale = static_cast<double>(page_rect.height()) /
                                 (root->get_shape()->depth() * Layout::dist_y + 2 * Layout::extent);
            double print_scale = std::min(new_x_scale, new_y_scale) * 100;
            if (print_scale < 1.0)
                print_scale = 1.0;
            if (print_scale > 400.0)
                print_scale = 400.0;
            print_scale = print_scale / 100.0;

            QPainter painter(&printer);
            painter.setRenderHint(QPainter::Antialiasing);
            painter.scale(print_scale, print_scale);
            painter.translate(xtrans, 0);
            QRect clip(0, 0, 0, 0);
            DrawingCursor dc(root, *na, cur_best, painter, clip, show_copies);
            PreorderNodeVisitor<DrawingCursor>(dc).run();
        }
    }

    VisualNode*
    TreeCanvas::event_node(QEvent* event) {
        int x = 0;
        int y = 0;
        switch (event->type()) {
        case QEvent::ToolTip: {
            QHelpEvent* he = static_cast<QHelpEvent*>(event);
            x = he->x();
            y = he->y();
            break;
        }
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseMove: {
            QMouseEvent* me = static_cast<QMouseEvent*>(event);
#if QT_VERSION >= 0x060000
            x = me->position().x();
            y = me->position().y();
#else
            x = me->x();
            y = me->y();
#endif
            break;
        }
        case QEvent::ContextMenu: {
            QContextMenuEvent* ce = static_cast<QContextMenuEvent*>(event);
            x = ce->x();
            y = ce->y();
            break;
        }
        default:
            return nullptr;
        }
        QAbstractScrollArea* sa = static_cast<QAbstractScrollArea*>(parentWidget()->parentWidget());
        int xoff = sa->horizontalScrollBar()->value() / scale;
        int yoff = sa->verticalScrollBar()->value() / scale;

        BoundingBox bb = root->get_bounding_box();
        int w = static_cast<int>((bb.right - bb.left + Layout::extent) * scale);
        if (w < sa->viewport()->width())
            xoff -= (sa->viewport()->width() - w) / 2;

        VisualNode* n;
        n = root->find_node(*na, static_cast<int>(x / scale - xtrans + xoff),
                            static_cast<int>((y - 30) / scale + yoff));
        return n;
    }

    bool
    TreeCanvas::event(QEvent* event) {
        if (mutex.tryLock()) {
            if (event->type() == QEvent::ToolTip) {
                VisualNode* n = event_node(event);
                if (n != nullptr) {
                    QHelpEvent* he = static_cast<QHelpEvent*>(event);
                    QToolTip::showText(he->globalPos(), QString(n->tool_tip(*na, cur_best, c_d, a_d).c_str()));
                } else {
                    QToolTip::hideText();
                }
            }
            mutex.unlock();
        }
        return QWidget::event(event);
    }

    void
    TreeCanvas::resize_to_outer(void) {
        if (auto_zoom)
            zoom_to_fit();
    }

    void
    TreeCanvas::paintEvent(QPaintEvent* event) {
        QMutexLocker locker(&layout_mutex);
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);

        QAbstractScrollArea* sa = static_cast<QAbstractScrollArea*>(parentWidget()->parentWidget());
        int xoff = sa->horizontalScrollBar()->value() / scale;
        int yoff = sa->verticalScrollBar()->value() / scale;

        BoundingBox bb = root->get_bounding_box();
        int w = static_cast<int>((bb.right - bb.left + Layout::extent) * scale);
        if (w < sa->viewport()->width())
            xoff -= (sa->viewport()->width() - w) / 2;

        QRect orig_clip = event->rect();
        painter.translate(0, 30);
        painter.scale(scale, scale);
        painter.translate(xtrans - xoff, -yoff);
        QRect clip(static_cast<int>(orig_clip.x() / scale - xtrans + xoff),
                   static_cast<int>(orig_clip.y() / scale + yoff), static_cast<int>(orig_clip.width() / scale),
                   static_cast<int>(orig_clip.height() / scale));
        DrawingCursor dc(root, *na, cur_best, painter, clip, show_copies);
        PreorderNodeVisitor<DrawingCursor>(dc).run();

        // int nodesLayouted = 1;
        // clock_t t0 = clock();
        // while (v.next()) { nodesLayouted++; }
        // double t = (static_cast<double>(clock()-t0) / CLOCKS_PER_SEC) * 1000.0;
        // double nps = static_cast<double>(nodesLayouted) /
        //   (static_cast<double>(clock()-t0) / CLOCKS_PER_SEC);
        // std::cout << "Drawing done. " << nodesLayouted << " nodes in "
        //   << t << " ms. " << nps << " nodes/s." << std::endl;
    }

    void
    TreeCanvas::mouseDoubleClickEvent(QMouseEvent* event) {
        if (mutex.tryLock()) {
            if (event->button() == Qt::LeftButton) {
                VisualNode* n = event_node(event);
                if (n == current_node) {
                    inspect_current_node();
                    event->accept();
                    mutex.unlock();
                    return;
                }
            }
            mutex.unlock();
        }
        event->ignore();
    }

    void
    TreeCanvas::contextMenuEvent(QContextMenuEvent* event) {
        if (mutex.tryLock()) {
            VisualNode* n = event_node(event);
            if (n != nullptr) {
                set_current_node(n);
                emit context_menu(event);
                event->accept();
                mutex.unlock();
                return;
            }
            mutex.unlock();
        }
        event->ignore();
    }

    void
    TreeCanvas::resizeEvent(QResizeEvent* e) {
        QAbstractScrollArea* sa = static_cast<QAbstractScrollArea*>(parentWidget()->parentWidget());

        int w = sa->horizontalScrollBar()->maximum() + e->oldSize().width();
        int h = sa->verticalScrollBar()->maximum() + e->oldSize().height();

        sa->horizontalScrollBar()->setRange(0, w - e->size().width());
        sa->verticalScrollBar()->setRange(0, h - e->size().height());
        sa->horizontalScrollBar()->setPageStep(e->size().width());
        sa->verticalScrollBar()->setPageStep(e->size().height());
    }

    void
    TreeCanvas::wheelEvent(QWheelEvent* event) {
        if (event->modifiers() & Qt::ShiftModifier) {
            event->accept();
#if QT_VERSION >= 0x060000
            if (event->angleDelta().y() != 0 && !auto_zoom)
                scale_tree(scale * 100 + ceil(static_cast<double>(event->angleDelta().y()) / 4.0),
                           event->position().x(), event->position().y());
#else
            if (event->orientation() == Qt::Vertical && !auto_zoom)
                scale_tree(scale * 100 + ceil(static_cast<double>(event->delta()) / 4.0), event->x(), event->y());
#endif
        } else {
            event->ignore();
        }
    }

    bool
    TreeCanvas::finish(void) {
        if (finished_flag)
            return true;
        stop_search_flag = true;
        finished_flag = true;
        for (int i = 0; i < double_click_inspectors.size(); i++)
            double_click_inspectors[i].first->finalize();
        for (int i = 0; i < solution_inspectors.size(); i++)
            solution_inspectors[i].first->finalize();
        for (int i = 0; i < move_inspectors.size(); i++)
            move_inspectors[i].first->finalize();
        for (int i = 0; i < comparators.size(); i++)
            comparators[i].first->finalize();
        return !searcher.isRunning();
    }

    void
    TreeCanvas::set_current_node(VisualNode* n, bool finished, bool update) {
        if (finished)
            mutex.lock();
        if (update && n != nullptr && n != current_node && n->get_status() != UNDETERMINED && !n->is_hidden()) {
            Space* cur_space = nullptr;
            for (int i = 0; i < move_inspectors.size(); i++) {
                if (move_inspectors[i].second) {
                    if (cur_space == nullptr)
                        cur_space = n->get_space(*na, cur_best, c_d, a_d);
                    try {
                        move_inspectors[i].first->inspect(*cur_space);
                    } catch (Exception& e) {
                        qFatal("Exception in move inspector %d: %s.\n Stopping.", i, e.what());
                    }
                }
            }
        }
        if (n != nullptr) {
            current_node->set_marked(false);
            current_node = n;
            current_node->set_marked(true);
            emit status_changed(current_node, stats, finished);
            if (update) {
                compare_nodes = false;
                setCursor(QCursor(Qt::ArrowCursor));
                QWidget::update();
            }
        }
        if (finished)
            mutex.unlock();
    }

    void
    TreeCanvas::mousePressEvent(QMouseEvent* event) {
        if (mutex.tryLock()) {
            if (event->button() == Qt::LeftButton) {
                VisualNode* n = event_node(event);
                if (compare_nodes) {
                    if (n != nullptr && n->get_status() != UNDETERMINED && current_node != nullptr &&
                        current_node->get_status() != UNDETERMINED) {
                        Space* cur_space = nullptr;
                        Space* compare_space = nullptr;
                        for (int i = 0; i < comparators.size(); i++) {
                            if (comparators[i].second) {
                                if (cur_space == nullptr) {
                                    cur_space = current_node->get_space(*na, cur_best, c_d, a_d);

                                    if (!compare_nodes_before_fp || n->is_root()) {
                                        compare_space = n->get_space(*na, cur_best, c_d, a_d);
                                    } else {
                                        VisualNode* p = n->get_parent(*na);
                                        compare_space = p->get_space(*na, cur_best, c_d, a_d);
                                        switch (compare_space->status()) {
                                        case SS_SOLVED:
                                        case SS_FAILED:
                                            break;
                                        case SS_BRANCH:
                                            compare_space->commit(*p->get_choice(), n->get_alternative(*na));
                                            break;
                                        default:
                                            GECODE_NEVER;
                                        }
                                    }
                                }
                                try {
                                    comparators[i].first->compare(*cur_space, *compare_space);
                                } catch (Exception& e) {
                                    qFatal("Exception in comparator %d: %s.\n Stopping.", i, e.what());
                                }
                            }
                        }
                    }
                } else {
                    set_current_node(n);
                }
                compare_nodes = false;
                setCursor(QCursor(Qt::ArrowCursor));
                if (n != nullptr) {
                    event->accept();
                    mutex.unlock();
                    return;
                }
            }
            mutex.unlock();
        }
        event->ignore();
    }

    void
    TreeCanvas::set_recomp_distances(int c_d0, int a_d0) {
        c_d = c_d0;
        a_d = a_d0;
    }

    void
    TreeCanvas::set_auto_hide_failed(bool b) {
        auto_hide_failed = b;
    }

    void
    TreeCanvas::set_auto_zoom(bool b) {
        auto_zoom = b;
        if (auto_zoom) {
            zoom_to_fit();
        }
        emit auto_zoom_changed(b);
        scale_bar->setEnabled(!b);
    }

    void
    TreeCanvas::set_show_copies(bool b) {
        show_copies = b;
    }
    bool
    TreeCanvas::get_show_copies(void) {
        return show_copies;
    }

    bool
    TreeCanvas::get_auto_hide_failed(void) {
        return auto_hide_failed;
    }

    bool
    TreeCanvas::get_auto_zoom(void) {
        return auto_zoom;
    }

    void
    TreeCanvas::set_refresh(int i) {
        refresh = i;
    }

    void
    TreeCanvas::set_refresh_pause(int i) {
        refresh_pause = i;
        if (refresh_pause > 0)
            refresh = 1;
    }

    bool
    TreeCanvas::get_smooth_scroll_and_zoom(void) {
        return smooth_scroll_and_zoom;
    }

    void
    TreeCanvas::set_smooth_scroll_and_zoom(bool b) {
        smooth_scroll_and_zoom = b;
    }

    bool
    TreeCanvas::get_move_during_search(void) {
        return move_during_search;
    }

    void
    TreeCanvas::set_move_during_search(bool b) {
        move_during_search = b;
    }

}} // namespace Gecode::Gist

// STATISTICS: gist-any
