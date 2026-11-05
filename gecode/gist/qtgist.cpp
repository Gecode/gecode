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

#include <gecode/gist/qtgist.hh>

#include <gecode/gist/zoomToFitIcon.hpp>
#include <gecode/gist/nodevisitor.hh>
#include <gecode/gist/nodecursor.hh>

namespace Gecode { namespace Gist {

    Gist::Gist(Space* root, bool bab, QWidget* parent, const Options& opt) : QWidget(parent) {
        QGridLayout* layout = new QGridLayout(this);

        QAbstractScrollArea* scroll_area = new QAbstractScrollArea(this);

        scroll_area->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        scroll_area->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        scroll_area->setAutoFillBackground(true);
        QPalette my_palette(scroll_area->palette());
        my_palette.setColor(QPalette::Window, Qt::white);
        scroll_area->setPalette(my_palette);
        canvas = new TreeCanvas(root, bab, scroll_area->viewport(), opt);
        canvas->setPalette(my_palette);
        canvas->setObjectName("canvas");

        connect(scroll_area->horizontalScrollBar(), SIGNAL(valueChanged(int)), canvas, SLOT(scroll()));
        connect(scroll_area->verticalScrollBar(), SIGNAL(valueChanged(int)), canvas, SLOT(scroll()));

        QVBoxLayout* sa_layout = new QVBoxLayout();
        sa_layout->setContentsMargins(0, 0, 0, 0);
        sa_layout->addWidget(canvas);
        scroll_area->viewport()->setLayout(sa_layout);

        connect(canvas, SIGNAL(solution(const Space*)), this, SIGNAL(solution(const Space*)));

        connect(canvas, SIGNAL(search_finished()), this, SIGNAL(search_finished()));

        QPixmap my_pic;
        my_pic.loadFromData(zoom_to_fit_icon, sizeof(zoom_to_fit_icon));

        QToolButton* auto_zoom_button = new QToolButton();
        auto_zoom_button->setCheckable(true);
        auto_zoom_button->setIcon(my_pic);

        node_stat_inspector = new NodeStatInspector(this);

        inspect = new QAction("Inspect", this);
        inspect->setShortcut(QKeySequence("Return"));
        connect(inspect, SIGNAL(triggered()), canvas, SLOT(inspect_current_node()));

        inspect_before_fp = new QAction("Inspect before fixpoint", this);
        inspect_before_fp->setShortcut(QKeySequence("Ctrl+Return"));
        connect(inspect_before_fp, SIGNAL(triggered()), canvas, SLOT(inspect_before_fp()));

        stop = new QAction("Stop search", this);
        stop->setShortcut(QKeySequence("Esc"));
        connect(stop, SIGNAL(triggered()), canvas, SLOT(stop_search()));

        reset = new QAction("Reset", this);
        reset->setShortcut(QKeySequence("Ctrl+R"));
        connect(reset, SIGNAL(triggered()), canvas, SLOT(reset()));

        nav_up = new QAction("Up", this);
        nav_up->setShortcut(QKeySequence("Up"));
        connect(nav_up, SIGNAL(triggered()), canvas, SLOT(nav_up()));

        nav_down = new QAction("Down", this);
        nav_down->setShortcut(QKeySequence("Down"));
        connect(nav_down, SIGNAL(triggered()), canvas, SLOT(nav_down()));

        nav_left = new QAction("Left", this);
        nav_left->setShortcut(QKeySequence("Left"));
        connect(nav_left, SIGNAL(triggered()), canvas, SLOT(nav_left()));

        nav_right = new QAction("Right", this);
        nav_right->setShortcut(QKeySequence("Right"));
        connect(nav_right, SIGNAL(triggered()), canvas, SLOT(nav_right()));

        nav_root = new QAction("Root", this);
        nav_root->setShortcut(QKeySequence("R"));
        connect(nav_root, SIGNAL(triggered()), canvas, SLOT(nav_root()));

        nav_next_sol = new QAction("To next solution", this);
        nav_next_sol->setShortcut(QKeySequence("Shift+Right"));
        connect(nav_next_sol, SIGNAL(triggered()), canvas, SLOT(nav_next_sol()));

        nav_prev_sol = new QAction("To previous solution", this);
        nav_prev_sol->setShortcut(QKeySequence("Shift+Left"));
        connect(nav_prev_sol, SIGNAL(triggered()), canvas, SLOT(nav_prev_sol()));

        search_next = new QAction("Next solution", this);
        search_next->setShortcut(QKeySequence("N"));
        connect(search_next, SIGNAL(triggered()), canvas, SLOT(search_one()));

        search_all = new QAction("All solutions", this);
        search_all->setShortcut(QKeySequence("A"));
        connect(search_all, SIGNAL(triggered()), canvas, SLOT(search_all()));

        toggle_hidden = new QAction("Hide/unhide", this);
        toggle_hidden->setShortcut(QKeySequence("H"));
        connect(toggle_hidden, SIGNAL(triggered()), canvas, SLOT(toggle_hidden()));

        hide_failed = new QAction("Hide failed subtrees", this);
        hide_failed->setShortcut(QKeySequence("F"));
        connect(hide_failed, SIGNAL(triggered()), canvas, SLOT(hide_failed()));

        unhide_all = new QAction("Unhide all", this);
        unhide_all->setShortcut(QKeySequence("U"));
        connect(unhide_all, SIGNAL(triggered()), canvas, SLOT(unhide_all()));

        label_branches = new QAction("Label/clear branches", this);
        label_branches->setShortcut(QKeySequence("L"));
        connect(label_branches, SIGNAL(triggered()), canvas, SLOT(label_branches()));

        label_path = new QAction("Label/clear path", this);
        label_path->setShortcut(QKeySequence("Shift+L"));
        connect(label_path, SIGNAL(triggered()), canvas, SLOT(label_path()));

        toggle_stop = new QAction("Stop/unstop", this);
        toggle_stop->setShortcut(QKeySequence("X"));
        connect(toggle_stop, SIGNAL(triggered()), canvas, SLOT(toggle_stop()));

        unstop_all = new QAction("Do not stop in subtree", this);
        unstop_all->setShortcut(QKeySequence("Shift+X"));
        connect(unstop_all, SIGNAL(triggered()), canvas, SLOT(unstop_all()));

        zoom_to_fit = new QAction("Zoom to fit", this);
        zoom_to_fit->setShortcut(QKeySequence("Z"));
        connect(zoom_to_fit, SIGNAL(triggered()), canvas, SLOT(zoom_to_fit()));

        center = new QAction("Center current node", this);
        center->setShortcut(QKeySequence("C"));
        connect(center, SIGNAL(triggered()), canvas, SLOT(center_current_node()));

        export_pdf = new QAction("Export subtree PDF...", this);
        export_pdf->setShortcut(QKeySequence("P"));
        connect(export_pdf, SIGNAL(triggered()), canvas, SLOT(export_pdf()));

        export_whole_tree_pdf = new QAction("Export PDF...", this);
        export_whole_tree_pdf->setShortcut(QKeySequence("Ctrl+Shift+P"));
        connect(export_whole_tree_pdf, SIGNAL(triggered()), canvas, SLOT(export_whole_tree_pdf()));

        print = new QAction("Print...", this);
        print->setShortcut(QKeySequence("Ctrl+P"));
        connect(print, SIGNAL(triggered()), canvas, SLOT(print()));

        bookmark_node = new QAction("Add/remove bookmark", this);
        bookmark_node->setShortcut(QKeySequence("Shift+B"));
        connect(bookmark_node, SIGNAL(triggered()), canvas, SLOT(bookmark_node()));

        compare_node = new QAction("Compare", this);
        compare_node->setShortcut(QKeySequence("V"));
        connect(compare_node, SIGNAL(triggered()), canvas, SLOT(start_compare_nodes()));

        compare_node_before_fp = new QAction("Compare before fixpoint", this);
        compare_node_before_fp->setShortcut(QKeySequence("Ctrl+V"));
        connect(compare_node_before_fp, SIGNAL(triggered()), canvas, SLOT(start_compare_nodes_before_fp()));

        connect(canvas, SIGNAL(added_bookmark(const QString&)), this, SLOT(add_bookmark(const QString&)));
        connect(canvas, SIGNAL(removed_bookmark(int)), this, SLOT(remove_bookmark(int)));

        null_bookmark = new QAction("<none>", this);
        null_bookmark->setCheckable(true);
        null_bookmark->setChecked(false);
        null_bookmark->setEnabled(false);
        bookmarks_group = new QActionGroup(this);
        bookmarks_group->setExclusive(false);
        bookmarks_group->addAction(null_bookmark);
        connect(bookmarks_group, SIGNAL(triggered(QAction*)), this, SLOT(select_bookmark(QAction*)));

        bookmarks_menu = new QMenu("Bookmarks");
        connect(bookmarks_menu, SIGNAL(aboutToShow()), this, SLOT(populate_bookmarks_menu()));

        set_path = new QAction("Set path", this);
        set_path->setShortcut(QKeySequence("Shift+P"));
        connect(set_path, SIGNAL(triggered()), canvas, SLOT(set_path()));

        inspect_path = new QAction("Inspect path", this);
        inspect_path->setShortcut(QKeySequence("Shift+I"));
        connect(inspect_path, SIGNAL(triggered()), canvas, SLOT(inspect_path()));

        show_node_stats = new QAction("Node statistics", this);
        show_node_stats->setShortcut(QKeySequence("S"));
        connect(show_node_stats, SIGNAL(triggered()), this, SLOT(show_stats()));

        addAction(inspect);
        addAction(inspect_before_fp);
        addAction(compare_node);
        addAction(compare_node_before_fp);
        addAction(stop);
        addAction(reset);
        addAction(nav_up);
        addAction(nav_down);
        addAction(nav_left);
        addAction(nav_right);
        addAction(nav_root);
        addAction(nav_next_sol);
        addAction(nav_prev_sol);

        addAction(search_next);
        addAction(search_all);
        addAction(toggle_hidden);
        addAction(hide_failed);
        addAction(unhide_all);
        addAction(label_branches);
        addAction(label_path);
        addAction(toggle_stop);
        addAction(unstop_all);
        addAction(zoom_to_fit);
        addAction(center);
        addAction(export_pdf);
        addAction(export_whole_tree_pdf);
        addAction(print);

        addAction(set_path);
        addAction(inspect_path);
        addAction(show_node_stats);

        null_solution_inspector = new QAction("<none>", this);
        null_solution_inspector->setCheckable(true);
        null_solution_inspector->setChecked(false);
        null_solution_inspector->setEnabled(false);
        solution_inspector_group = new QActionGroup(this);
        solution_inspector_group->setExclusive(false);
        solution_inspector_group->addAction(null_solution_inspector);
        connect(solution_inspector_group, SIGNAL(triggered(QAction*)), this, SLOT(select_solution_inspector(QAction*)));

        null_double_click_inspector = new QAction("<none>", this);
        null_double_click_inspector->setCheckable(true);
        null_double_click_inspector->setChecked(false);
        null_double_click_inspector->setEnabled(false);
        double_click_inspector_group = new QActionGroup(this);
        double_click_inspector_group->setExclusive(false);
        double_click_inspector_group->addAction(null_double_click_inspector);
        connect(double_click_inspector_group, SIGNAL(triggered(QAction*)), this,
                SLOT(select_double_click_inspector(QAction*)));

        null_move_inspector = new QAction("<none>", this);
        null_move_inspector->setCheckable(true);
        null_move_inspector->setChecked(false);
        null_move_inspector->setEnabled(false);
        move_inspector_group = new QActionGroup(this);
        move_inspector_group->setExclusive(false);
        move_inspector_group->addAction(null_move_inspector);
        connect(move_inspector_group, SIGNAL(triggered(QAction*)), this, SLOT(select_move_inspector(QAction*)));

        null_comparator = new QAction("<none>", this);
        null_comparator->setCheckable(true);
        null_comparator->setChecked(false);
        null_comparator->setEnabled(false);
        comparator_group = new QActionGroup(this);
        comparator_group->setExclusive(false);
        comparator_group->addAction(null_comparator);
        connect(comparator_group, SIGNAL(triggered(QAction*)), this, SLOT(select_comparator(QAction*)));

        solution_inspector_menu = new QMenu("Solution inspectors");
        solution_inspector_menu->addActions(solution_inspector_group->actions());
        double_click_inspector_menu = new QMenu("Double click inspectors");
        double_click_inspector_menu->addActions(double_click_inspector_group->actions());
        move_inspector_menu = new QMenu("Move inspectors");
        move_inspector_menu->addActions(move_inspector_group->actions());
        comparator_menu = new QMenu("Comparators");
        comparator_menu->addActions(comparator_group->actions());

        inspect_group = new QActionGroup(this);
        connect(inspect_group, SIGNAL(triggered(QAction*)), this, SLOT(inspect_with_action(QAction*)));
        inspect_before_fp_group = new QActionGroup(this);
        connect(inspect_before_fp_group, SIGNAL(triggered(QAction*)), this,
                SLOT(inspect_before_fp_with_action(QAction*)));

        inspect_node_menu = new QMenu("Inspect");
        inspect_node_menu->addAction(inspect);
        connect(inspect_node_menu, SIGNAL(aboutToShow()), this, SLOT(populate_inspectors()));

        inspect_node_before_fp_menu = new QMenu("Inspect before fixpoint");
        inspect_node_before_fp_menu->addAction(inspect_before_fp);
        connect(inspect_node_before_fp_menu, SIGNAL(aboutToShow()), this, SLOT(populate_inspectors()));
        populate_inspectors();

        context_menu = new QMenu(this);
        context_menu->addMenu(inspect_node_menu);
        context_menu->addMenu(inspect_node_before_fp_menu);
        context_menu->addAction(compare_node);
        context_menu->addAction(compare_node_before_fp);
        context_menu->addAction(show_node_stats);
        context_menu->addAction(center);

        context_menu->addSeparator();

        context_menu->addAction(search_next);
        context_menu->addAction(search_all);

        context_menu->addSeparator();

        context_menu->addAction(toggle_hidden);
        context_menu->addAction(hide_failed);
        context_menu->addAction(unhide_all);
        context_menu->addAction(label_branches);
        context_menu->addAction(label_path);

        context_menu->addAction(toggle_stop);
        context_menu->addAction(unstop_all);

        context_menu->addSeparator();

        context_menu->addMenu(bookmarks_menu);
        context_menu->addAction(set_path);
        context_menu->addAction(inspect_path);

        context_menu->addSeparator();

        context_menu->addMenu(double_click_inspector_menu);
        context_menu->addMenu(solution_inspector_menu);
        context_menu->addMenu(move_inspector_menu);

        connect(auto_zoom_button, SIGNAL(toggled(bool)), canvas, SLOT(set_auto_zoom(bool)));

        connect(canvas, SIGNAL(auto_zoom_changed(bool)), auto_zoom_button, SLOT(setChecked(bool)));

        {
            unsigned int i = 0;
            while (opt.inspect.solution(i)) {
                add_solution_inspector(opt.inspect.solution(i++));
            }
            i = 0;
            while (opt.inspect.click(i)) {
                add_double_click_inspector(opt.inspect.click(i++));
            }
            i = 0;
            while (opt.inspect.move(i)) {
                add_move_inspector(opt.inspect.move(i++));
            }
            i = 0;
            while (opt.inspect.compare(i)) {
                add_comparator(opt.inspect.compare(i++));
            }
        }

        layout->addWidget(scroll_area, 0, 0, -1, 1);
        layout->addWidget(canvas->scale_bar, 1, 1, Qt::AlignHCenter);
        layout->addWidget(auto_zoom_button, 0, 1, Qt::AlignHCenter);

        setLayout(layout);

        canvas->show();

        resize(500, 400);

        // enables on_<sender>_<signal>() mechanism
        QMetaObject::connectSlotsByName(this);
    }

    void
    Gist::resizeEvent(QResizeEvent*) {
        canvas->resize_to_outer();
    }

    void
    Gist::add_inspector(Inspector* i0, QAction*& nas, QAction*& nad, QAction*& nam) {
        if (double_click_inspector_group->actions().indexOf(null_double_click_inspector) != -1) {
            double_click_inspector_group->removeAction(null_double_click_inspector);
            solution_inspector_group->removeAction(null_solution_inspector);
            move_inspector_group->removeAction(null_move_inspector);
        }
        canvas->add_solution_inspector(i0);
        canvas->add_double_click_inspector(i0);
        canvas->add_move_inspector(i0);

        nas = new QAction(i0->name().c_str(), this);
        nas->setCheckable(true);
        solution_inspector_group->addAction(nas);
        solution_inspector_menu->clear();
        solution_inspector_menu->addActions(solution_inspector_group->actions());

        nad = new QAction(i0->name().c_str(), this);
        nad->setCheckable(true);
        double_click_inspector_group->addAction(nad);
        double_click_inspector_menu->clear();
        double_click_inspector_menu->addActions(double_click_inspector_group->actions());

        nam = new QAction(i0->name().c_str(), this);
        nam->setCheckable(true);
        move_inspector_group->addAction(nam);
        move_inspector_menu->clear();
        move_inspector_menu->addActions(move_inspector_group->actions());

        QAction* ia = new QAction(i0->name().c_str(), this);
        inspect_group->addAction(ia);
        QAction* ibfpa = new QAction(i0->name().c_str(), this);
        inspect_before_fp_group->addAction(ibfpa);

        if (inspect_group->actions().size() < 10) {
            ia->setShortcut(QKeySequence(QString("Ctrl+") + QString("").setNum(inspect_group->actions().size())));
            ibfpa->setShortcut(
                QKeySequence(QString("Ctrl+Alt+") + QString("").setNum(inspect_before_fp_group->actions().size())));
        }
    }

    void
    Gist::add_solution_inspector(Inspector* ins) {
        QAction* nas;
        QAction* nad;
        QAction* nam;
        if (double_click_inspector_group->actions().indexOf(null_double_click_inspector) == -1) {
            QList<QAction*> is = solution_inspector_group->actions();
            for (int i = 0; i < is.size(); i++) {
                canvas->activate_solution_inspector(i, false);
                is[i]->setChecked(false);
            }
        }
        add_inspector(ins, nas, nad, nam);
        nas->setChecked(true);
        select_solution_inspector(nas);
    }

    void
    Gist::add_double_click_inspector(Inspector* ins) {
        QAction* nas;
        QAction* nad;
        QAction* nam;
        if (double_click_inspector_group->actions().indexOf(null_double_click_inspector) == -1) {
            QList<QAction*> is = double_click_inspector_group->actions();
            for (int i = 0; i < is.size(); i++) {
                canvas->activate_double_click_inspector(i, false);
                is[i]->setChecked(false);
            }
        }
        add_inspector(ins, nas, nad, nam);
        nad->setChecked(true);
        select_double_click_inspector(nad);
    }

    void
    Gist::add_move_inspector(Inspector* ins) {
        QAction* nas;
        QAction* nad;
        QAction* nam;
        if (double_click_inspector_group->actions().indexOf(null_double_click_inspector) == -1) {
            QList<QAction*> is = move_inspector_group->actions();
            for (int i = 0; i < is.size(); i++) {
                canvas->activate_move_inspector(i, false);
                is[i]->setChecked(false);
            }
        }
        add_inspector(ins, nas, nad, nam);
        nam->setChecked(true);
        select_move_inspector(nam);
    }

    void
    Gist::add_comparator(Comparator* c) {
        if (comparator_group->actions().indexOf(null_comparator) == -1) {
            QList<QAction*> is = comparator_group->actions();
            for (int i = 0; i < is.size(); i++) {
                canvas->activate_comparator(i, false);
                is[i]->setChecked(false);
            }
        } else {
            comparator_group->removeAction(null_comparator);
        }
        canvas->add_comparator(c);

        QAction* ncs = new QAction(c->name().c_str(), this);
        ncs->setCheckable(true);
        comparator_group->addAction(ncs);
        comparator_menu->clear();
        comparator_menu->addActions(comparator_group->actions());
        ncs->setChecked(true);
        select_comparator(ncs);
    }

    Gist::~Gist() {
        delete canvas;
    }

    void
    Gist::on_canvas_context_menu(QContextMenuEvent* event) {
        context_menu->popup(event->globalPos());
    }

    void
    Gist::on_canvas_status_changed(VisualNode* n, const Statistics& stats, bool finished) {
        node_stat_inspector->node(*canvas->na, n, stats, finished);
        if (!finished) {
            inspect->setEnabled(false);
            inspect_group->setEnabled(false);
            inspect_before_fp->setEnabled(false);
            inspect_before_fp_group->setEnabled(false);
            compare_node->setEnabled(false);
            compare_node_before_fp->setEnabled(false);
            show_node_stats->setEnabled(false);
            stop->setEnabled(true);
            reset->setEnabled(false);
            nav_up->setEnabled(false);
            nav_down->setEnabled(false);
            nav_left->setEnabled(false);
            nav_right->setEnabled(false);
            nav_root->setEnabled(false);
            nav_next_sol->setEnabled(false);
            nav_prev_sol->setEnabled(false);

            search_next->setEnabled(false);
            search_all->setEnabled(false);
            toggle_hidden->setEnabled(false);
            hide_failed->setEnabled(false);
            unhide_all->setEnabled(false);
            label_branches->setEnabled(false);
            label_path->setEnabled(false);

            toggle_stop->setEnabled(false);
            unstop_all->setEnabled(false);

            center->setEnabled(false);
            export_pdf->setEnabled(false);
            export_whole_tree_pdf->setEnabled(false);
            print->setEnabled(false);

            set_path->setEnabled(false);
            inspect_path->setEnabled(false);
            bookmark_node->setEnabled(false);
            bookmarks_group->setEnabled(false);
        } else {
            stop->setEnabled(false);
            reset->setEnabled(true);

            if ((n->is_open() || n->has_open_children()) && (!n->is_hidden())) {
                search_next->setEnabled(true);
                search_all->setEnabled(true);
            } else {
                search_next->setEnabled(false);
                search_all->setEnabled(false);
            }
            if (n->get_number_of_children() > 0) {
                nav_down->setEnabled(true);
                toggle_hidden->setEnabled(true);
                hide_failed->setEnabled(true);
                unhide_all->setEnabled(true);
                unstop_all->setEnabled(true);
            } else {
                nav_down->setEnabled(false);
                toggle_hidden->setEnabled(false);
                hide_failed->setEnabled(false);
                unhide_all->setEnabled(false);
                unstop_all->setEnabled(false);
            }

            toggle_stop->setEnabled(n->get_status() == STOP || n->get_status() == UNSTOP);

            show_node_stats->setEnabled(true);
            inspect->setEnabled(true);
            label_path->setEnabled(true);
            if (n->get_status() == UNDETERMINED) {
                inspect_group->setEnabled(false);
                inspect_before_fp->setEnabled(false);
                inspect_before_fp_group->setEnabled(false);
                compare_node->setEnabled(false);
                compare_node_before_fp->setEnabled(false);
                label_branches->setEnabled(false);
            } else {
                inspect_group->setEnabled(true);
                inspect_before_fp->setEnabled(true);
                inspect_before_fp_group->setEnabled(true);
                compare_node->setEnabled(true);
                compare_node_before_fp->setEnabled(true);
                label_branches->setEnabled(!n->is_hidden());
            }

            nav_root->setEnabled(true);
            VisualNode* p = n->get_parent(*canvas->na);
            if (p == nullptr) {
                inspect_before_fp->setEnabled(false);
                inspect_before_fp_group->setEnabled(false);
                nav_up->setEnabled(false);
                nav_right->setEnabled(false);
                nav_left->setEnabled(false);
            } else {
                nav_up->setEnabled(true);
                unsigned int alt = n->get_alternative(*canvas->na);
                nav_right->setEnabled(alt + 1 < p->get_number_of_children());
                nav_left->setEnabled(alt > 0);
            }

            VisualNode* root = n;
            while (!root->is_root())
                root = root->get_parent(*canvas->na);
            NextSolCursor nsc(n, false, *canvas->na);
            PreorderNodeVisitor<NextSolCursor> nsv(nsc);
            nsv.run();
            nav_next_sol->setEnabled(nsv.get_cursor().node() != root);

            NextSolCursor psc(n, true, *canvas->na);
            PreorderNodeVisitor<NextSolCursor> psv(psc);
            psv.run();
            nav_prev_sol->setEnabled(psv.get_cursor().node() != root);

            center->setEnabled(true);
            export_pdf->setEnabled(true);
            export_whole_tree_pdf->setEnabled(true);
            print->setEnabled(true);

            set_path->setEnabled(true);
            inspect_path->setEnabled(true);

            bookmark_node->setEnabled(true);
            bookmarks_group->setEnabled(true);
        }
        emit status_changed(stats, finished);
    }

    void
    Gist::inspect_with_action(QAction* a) {
        canvas->inspect_current_node(true, inspect_group->actions().indexOf(a));
    }

    void
    Gist::inspect_before_fp_with_action(QAction* a) {
        canvas->inspect_current_node(false, inspect_before_fp_group->actions().indexOf(a));
    }

    bool
    Gist::finish() {
        return canvas->finish();
    }

    void
    Gist::select_double_click_inspector(QAction* a) {
        canvas->activate_double_click_inspector(double_click_inspector_group->actions().indexOf(a), a->isChecked());
    }
    void
    Gist::select_solution_inspector(QAction* a) {
        canvas->activate_solution_inspector(solution_inspector_group->actions().indexOf(a), a->isChecked());
    }
    void
    Gist::select_move_inspector(QAction* a) {
        canvas->activate_move_inspector(move_inspector_group->actions().indexOf(a), a->isChecked());
    }
    void
    Gist::select_comparator(QAction* a) {
        canvas->activate_comparator(comparator_group->actions().indexOf(a), a->isChecked());
    }
    void
    Gist::select_bookmark(QAction* a) {
        int idx = bookmarks_group->actions().indexOf(a);
        canvas->set_current_node(canvas->bookmarks[idx]);
        canvas->center_current_node();
    }

    void
    Gist::add_bookmark(const QString& id) {
        if (bookmarks_group->actions().indexOf(null_bookmark) != -1) {
            bookmarks_group->removeAction(null_bookmark);
        }

        QAction* nb = new QAction(id, this);
        nb->setCheckable(true);
        bookmarks_group->addAction(nb);
    }

    void
    Gist::remove_bookmark(int idx) {
        QAction* a = bookmarks_group->actions()[idx];
        bookmarks_group->removeAction(a);
        if (bookmarks_group->actions().size() == 0) {
            bookmarks_group->addAction(null_bookmark);
        }
    }

    void
    Gist::populate_bookmarks_menu() {
        bookmarks_menu->clear();
        bookmarks_menu->addAction(bookmark_node);
        bookmarks_menu->addSeparator();
        bookmarks_menu->addActions(bookmarks_group->actions());
    }

    void
    Gist::populate_inspectors() {
        inspect_node_menu->clear();
        inspect_node_menu->addAction(inspect);
        inspect_node_menu->addSeparator();
        inspect_node_menu->addActions(inspect_group->actions());
        inspect_node_before_fp_menu->clear();
        inspect_node_before_fp_menu->addAction(inspect_before_fp);
        inspect_node_before_fp_menu->addSeparator();
        inspect_node_before_fp_menu->addActions(inspect_before_fp_group->actions());
    }

    void
    Gist::set_auto_hide_failed(bool b) {
        canvas->set_auto_hide_failed(b);
    }
    void
    Gist::set_auto_zoom(bool b) {
        canvas->set_auto_zoom(b);
    }
    bool
    Gist::get_auto_hide_failed() {
        return canvas->get_auto_hide_failed();
    }
    bool
    Gist::get_auto_zoom() {
        return canvas->get_auto_zoom();
    }
    void
    Gist::set_refresh(int i) {
        canvas->set_refresh(i);
    }
    void
    Gist::set_refresh_pause(int i) {
        canvas->set_refresh_pause(i);
    }
    bool
    Gist::get_smooth_scroll_and_zoom() {
        return canvas->get_smooth_scroll_and_zoom();
    }
    void
    Gist::set_smooth_scroll_and_zoom(bool b) {
        canvas->set_smooth_scroll_and_zoom(b);
    }
    bool
    Gist::get_move_during_search() {
        return canvas->get_move_during_search();
    }
    void
    Gist::set_move_during_search(bool b) {
        canvas->set_move_during_search(b);
    }
    void
    Gist::set_recomp_distances(int c_d, int a_d) {
        canvas->set_recomp_distances(c_d, a_d);
    }

    int
    Gist::get_cd() {
        return canvas->c_d;
    }
    int
    Gist::get_ad() {
        return canvas->a_d;
    }

    void
    Gist::set_show_copies(bool b) {
        canvas->set_show_copies(b);
    }
    bool
    Gist::get_show_copies() {
        return canvas->get_show_copies();
    }

    void
    Gist::show_stats() {
        node_stat_inspector->show_stats();
        canvas->emit_status_changed();
    }

}} // namespace Gecode::Gist

// STATISTICS: gist-any
