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

#include <gecode/gist/mainwindow.hh>
#include <gecode/gist/preferences.hh>
#include <gecode/gist/nodewidget.hh>
#include <gecode/gist/drawingcursor.hh>

#include <gecode/gist/gecodelogo.hh>

namespace Gecode::Gist {

    AboutGist::AboutGist(QWidget* parent) : QDialog(parent) {

        Logos logos;
        QPixmap my_pic;
        my_pic.loadFromData(logos.logo, logos.logo_size);

        QPixmap my_pic2;
        my_pic2.loadFromData(logos.gist_logo, logos.gist_logo_size);
        setWindowIcon(my_pic2);

        setMinimumSize(300, 240);
        setMaximumSize(300, 240);
        QVBoxLayout* layout = new QVBoxLayout();
        QLabel* logo = new QLabel();
        logo->setPixmap(my_pic);
        layout->addWidget(logo, 0, Qt::AlignCenter);
        QLabel* about_label = new QLabel(tr("<h2>Gist</h2>"
                                            "<p><b>The Gecode Interactive Search Tool</b</p> "
                                            "<p>You can find more information about Gecode and Gist "
                                            "at</p>"
                                            "<p><a href='http://www.gecode.dev'>www.gecode.dev</a>"
                                            "</p"));
        about_label->setOpenExternalLinks(true);
        about_label->setWordWrap(true);
        about_label->setAlignment(Qt::AlignCenter);
        layout->addWidget(about_label);
        setLayout(layout);
        setWindowTitle(tr("About Gist"));
        setAttribute(Qt::WA_QuitOnClose, false);
        setAttribute(Qt::WA_DeleteOnClose, false);
    }

    GistMainWindow::GistMainWindow(Space* root, bool bab, const Options& opt0) : opt(opt0), about_gist(this) {
        c = new Gist(root, bab, this, opt);
        setCentralWidget(c);
        setWindowTitle(tr("Gist"));

        Logos logos;
        QPixmap my_pic;
        my_pic.loadFromData(logos.gist_logo, logos.gist_logo_size);
        setWindowIcon(my_pic);

        resize(500, 500);
        setMinimumSize(400, 200);

        menu_bar = new QMenuBar(nullptr);

        QMenu* file_menu = menu_bar->addMenu(tr("&File"));
        file_menu->addAction(c->print);
#if QT_VERSION >= 0x040400
        file_menu->addAction(c->export_whole_tree_pdf);
#endif
        QAction* quit_action = file_menu->addAction(tr("Quit"));
        quit_action->setShortcut(QKeySequence("Ctrl+Q"));
        connect(quit_action, SIGNAL(triggered()), this, SLOT(close()));
        pref_action = file_menu->addAction(tr("Preferences"));
        connect(pref_action, SIGNAL(triggered()), this, SLOT(preferences()));

        QMenu* node_menu = menu_bar->addMenu(tr("&Node"));

        inspect_node_menu = new QMenu("Inspect");
        inspect_node_menu->addAction(c->inspect);
        connect(inspect_node_menu, SIGNAL(aboutToShow()), this, SLOT(populate_inspectors()));

        inspect_node_before_fp_menu = new QMenu("Inspect before fixpoint");
        inspect_node_before_fp_menu->addAction(c->inspect_before_fp);
        connect(inspect_node_before_fp_menu, SIGNAL(aboutToShow()), this, SLOT(populate_inspectors()));
        populate_inspectors();

        node_menu->addMenu(inspect_node_menu);
        node_menu->addMenu(inspect_node_before_fp_menu);
        node_menu->addAction(c->compare_node);
        node_menu->addAction(c->compare_node_before_fp);
        node_menu->addAction(c->set_path);
        node_menu->addAction(c->inspect_path);
        node_menu->addAction(c->show_node_stats);
        bookmarks_menu = new QMenu("Bookmarks");
        bookmarks_menu->addAction(c->bookmark_node);
        connect(bookmarks_menu, SIGNAL(aboutToShow()), this, SLOT(populate_bookmarks()));
        node_menu->addMenu(bookmarks_menu);
        node_menu->addSeparator();
        node_menu->addAction(c->nav_up);
        node_menu->addAction(c->nav_down);
        node_menu->addAction(c->nav_left);
        node_menu->addAction(c->nav_right);
        node_menu->addAction(c->nav_root);
        node_menu->addAction(c->nav_next_sol);
        node_menu->addAction(c->nav_prev_sol);
        node_menu->addSeparator();
        node_menu->addAction(c->toggle_hidden);
        node_menu->addAction(c->hide_failed);
        node_menu->addAction(c->unhide_all);
        node_menu->addAction(c->label_branches);
        node_menu->addAction(c->label_path);
        node_menu->addAction(c->toggle_stop);
        node_menu->addAction(c->unstop_all);
        node_menu->addSeparator();
        node_menu->addAction(c->zoom_to_fit);
        node_menu->addAction(c->center);
#if QT_VERSION >= 0x040400
        node_menu->addAction(c->export_pdf);
#endif

        QMenu* search_menu = menu_bar->addMenu(tr("&Search"));
        search_menu->addAction(c->search_next);
        search_menu->addAction(c->search_all);
        search_menu->addSeparator();
        search_menu->addAction(c->stop);
        search_menu->addSeparator();
        search_menu->addAction(c->reset);

        QMenu* tools_menu = menu_bar->addMenu(tr("&Tools"));
        double_click_inspectors_menu = new QMenu("Double click Inspectors");
        connect(double_click_inspectors_menu, SIGNAL(aboutToShow()), this, SLOT(populate_inspector_selection()));
        tools_menu->addMenu(double_click_inspectors_menu);
        solution_inspectors_menu = new QMenu("Solution inspectors");
        connect(solution_inspectors_menu, SIGNAL(aboutToShow()), this, SLOT(populate_inspector_selection()));
        tools_menu->addMenu(solution_inspectors_menu);
        move_inspectors_menu = new QMenu("Move inspectors");
        connect(move_inspectors_menu, SIGNAL(aboutToShow()), this, SLOT(populate_inspector_selection()));
        tools_menu->addMenu(move_inspectors_menu);
        comparators_menu = new QMenu("Comparators");
        connect(comparators_menu, SIGNAL(aboutToShow()), this, SLOT(populate_inspector_selection()));
        tools_menu->addMenu(comparators_menu);

        QMenu* help_menu = menu_bar->addMenu(tr("&Help"));
        QAction* about_action = help_menu->addAction(tr("About"));
        connect(about_action, SIGNAL(triggered()), this, SLOT(about()));

        // Don't add the menu bar on Mac OS X
#ifndef Q_WS_MAC
        setMenuBar(menu_bar);
#endif

        // Set up status bar
        QWidget* stw = new QWidget();
        QHBoxLayout* hbl = new QHBoxLayout();
        hbl->setContentsMargins(0, 0, 0, 0);
        hbl->addWidget(new QLabel("Depth:"));
        depth_label = new QLabel("0");
        hbl->addWidget(depth_label);
        hbl->addWidget(new NodeWidget(SOLVED));
        solved_label = new QLabel("0");
        hbl->addWidget(solved_label);
        hbl->addWidget(new NodeWidget(FAILED));
        failed_label = new QLabel("0");
        hbl->addWidget(failed_label);
        hbl->addWidget(new NodeWidget(BRANCH));
        choices_label = new QLabel("0");
        hbl->addWidget(choices_label);
        hbl->addWidget(new NodeWidget(UNDETERMINED));
        open_label = new QLabel("     0");
        hbl->addWidget(open_label);
        stw->setLayout(hbl);
        statusBar()->addPermanentWidget(stw);

        is_searching = false;
        statusBar()->showMessage("Ready");

        connect(c, SIGNAL(status_changed(const Statistics&, bool)), this,
                SLOT(status_changed(const Statistics&, bool)));

        connect(c, SIGNAL(search_finished()), this, SLOT(close()));

        preferences(true);
        show();
        c->reset->trigger();
    }

    void
    GistMainWindow::closeEvent(QCloseEvent* event) {
        if (c->finish())
            event->accept();
        else
            event->ignore();
    }

    void
    GistMainWindow::status_changed(const Statistics& stats, bool finished) {
        if (stats.max_depth == 0) {
            is_searching = false;
            statusBar()->showMessage("Ready");
            pref_action->setEnabled(true);
        } else if (is_searching && finished) {
            is_searching = false;
            double ms = search_timer.stop();
            double s = std::floor(ms / 1000.0);
            ms -= s * 1000.0;
            double m = std::floor(s / 60.0);
            s -= m * 60.0;
            double h = std::floor(m / 60.0);
            m -= h * 60.0;

            // QString t;
            // if (static_cast<int>(h) != 0)
            //   t += QString().setNum(static_cast<int>(h))+"h ";
            // if (static_cast<int>(m) != 0)
            //   t += QString().setNum(static_cast<int>(m))+"m ";
            // if (static_cast<int>(s) != 0)
            //   t += QString().setNum(static_cast<int>(s));
            // else
            //   t += "0";
            // t += "."+QString().setNum(static_cast<int>(ms))+"s";
            // statusBar()->showMessage(QString("Ready (search time ")+t+")");
            statusBar()->showMessage("Ready");
            pref_action->setEnabled(true);
        } else if (!is_searching && !finished) {
            pref_action->setEnabled(false);
            statusBar()->showMessage("Searching");
            is_searching = true;
            search_timer.start();
        }
        depth_label->setNum(stats.max_depth);
        solved_label->setNum(stats.solutions);
        failed_label->setNum(stats.failures);
        choices_label->setNum(stats.choices);
        open_label->setNum(stats.undetermined);
    }

    void
    GistMainWindow::about() {
        about_gist.show();
    }

    void
    GistMainWindow::preferences(bool setup) {
        PreferencesDialog pd(opt, this);
        if (setup) {
            c->set_auto_zoom(pd.zoom);
        }
        if (setup || pd.exec() == QDialog::Accepted) {
            c->set_auto_hide_failed(pd.hide_failed);
            c->set_refresh(pd.refresh);
            c->set_refresh_pause(pd.refresh_pause);
            c->set_smooth_scroll_and_zoom(pd.smooth_scroll_and_zoom);
            c->set_move_during_search(pd.move_during_search);
            c->set_recomp_distances(pd.c_d, pd.a_d);
            opt.c_d = pd.c_d;
            opt.a_d = pd.a_d;
            c->set_show_copies(pd.copies);
        }
    }

    void
    GistMainWindow::populate_inspector_selection() {
        double_click_inspectors_menu->clear();
        double_click_inspectors_menu->addActions(c->double_click_inspector_group->actions());
        solution_inspectors_menu->clear();
        solution_inspectors_menu->addActions(c->solution_inspector_group->actions());
        move_inspectors_menu->clear();
        move_inspectors_menu->addActions(c->move_inspector_group->actions());
        comparators_menu->clear();
        comparators_menu->addActions(c->comparator_group->actions());
    }

    void
    GistMainWindow::populate_bookmarks() {
        bookmarks_menu->clear();
        bookmarks_menu->addAction(c->bookmark_node);
        bookmarks_menu->addSeparator();
        bookmarks_menu->addActions(c->bookmarks_group->actions());
    }

    void
    GistMainWindow::populate_inspectors() {
        inspect_node_menu->clear();
        inspect_node_menu->addAction(c->inspect);
        inspect_node_menu->addSeparator();
        inspect_node_menu->addActions(c->inspect_group->actions());
        inspect_node_before_fp_menu->clear();
        inspect_node_before_fp_menu->addAction(c->inspect_before_fp);
        inspect_node_before_fp_menu->addSeparator();
        inspect_node_before_fp_menu->addActions(c->inspect_before_fp_group->actions());
    }

} // namespace Gecode::Gist

// STATISTICS: gist-any
