/*
 *  Main authors:
 *     Guido Tack <tack@gecode.dev>
 *
 *  Copyright:
 *     Guido Tack, 2007
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

#include <gecode/gist/preferences.hh>

namespace Gecode { namespace Gist {

    PreferencesDialog::PreferencesDialog(const Options& opt, QWidget* parent) : QDialog(parent) {
        QSettings settings("gecode.dev", "Gist");
        hide_failed = settings.value("search/hideFailed", true).toBool();
        zoom = settings.value("search/zoom", false).toBool();
        copies = settings.value("search/copies", false).toBool();
        refresh = settings.value("search/refresh", 500).toInt();
        refresh_pause = settings.value("search/refreshPause", 0).toInt();
        smooth_scroll_and_zoom = settings.value("smoothScrollAndZoom", true).toBool();
        move_during_search = false;

        c_d = opt.c_d;
        a_d = opt.a_d;

        hide_check = new QCheckBox(tr("Hide failed subtrees automatically"));
        hide_check->setChecked(hide_failed);
        zoom_check = new QCheckBox(tr("Automatic zoom enabled on start-up"));
        zoom_check->setChecked(zoom);
        smooth_check = new QCheckBox(tr("Smooth scrolling and zooming"));
        smooth_check->setChecked(smooth_scroll_and_zoom);

        QPushButton* def_button = new QPushButton(tr("Defaults"));
        QPushButton* cancel_button = new QPushButton(tr("Cancel"));
        QPushButton* ok_button = new QPushButton(tr("Ok"));
        ok_button->setDefault(true);
        QHBoxLayout* button_layout = new QHBoxLayout();
        button_layout->addWidget(def_button);
        button_layout->addWidget(cancel_button);
        button_layout->addWidget(ok_button);

        connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));
        connect(def_button, SIGNAL(clicked()), this, SLOT(defaults()));
        connect(ok_button, SIGNAL(clicked()), this, SLOT(write_back()));

        QLabel* refresh_label = new QLabel(tr("Display refresh rate:"));
        refresh_box = new QSpinBox();
        refresh_box->setRange(0, 1000000);
        refresh_box->setValue(refresh);
        refresh_box->setSingleStep(100);
        QHBoxLayout* refresh_layout = new QHBoxLayout();
        refresh_layout->addWidget(refresh_label);
        refresh_layout->addWidget(refresh_box);

        slow_box = new QCheckBox(tr("Slow down search"));
        slow_box->setChecked(refresh_pause > 0);

        refresh_box->setEnabled(refresh_pause == 0);

        connect(slow_box, SIGNAL(stateChanged(int)), this, SLOT(toggle_slow(int)));

        move_during_search_box = new QCheckBox(tr("Move cursor during search"));
        move_during_search_box->setChecked(move_during_search);

        QVBoxLayout* layout = new QVBoxLayout();
        layout->addWidget(hide_check);
        layout->addWidget(zoom_check);
        layout->addWidget(smooth_check);
        layout->addLayout(refresh_layout);
        layout->addWidget(slow_box);
        layout->addWidget(move_during_search_box);

        QTabWidget* tabs = new QTabWidget;
        QWidget* page1 = new QWidget;
        page1->setLayout(layout);
        tabs->addTab(page1, "Drawing");

        QLabel* cdlabel = new QLabel(tr("Commit distance:"));
        cd_box = new QSpinBox();
        cd_box->setRange(0, 10000);
        cd_box->setValue(c_d);
        cd_box->setSingleStep(1);
        QHBoxLayout* cd_layout = new QHBoxLayout();
        cd_layout->addWidget(cdlabel);
        cd_layout->addWidget(cd_box);
        QLabel* adlabel = new QLabel(tr("Adaptive distance:"));
        ad_box = new QSpinBox();
        ad_box->setRange(0, 10000);
        ad_box->setValue(a_d);
        ad_box->setSingleStep(1);
        QHBoxLayout* ad_layout = new QHBoxLayout();
        ad_layout->addWidget(adlabel);
        ad_layout->addWidget(ad_box);
        copies_check = new QCheckBox(tr("Show clones in the tree"));
        copies_check->setChecked(copies);
        layout = new QVBoxLayout();
        layout->addLayout(cd_layout);
        layout->addLayout(ad_layout);
        layout->addWidget(copies_check);
        QWidget* page2 = new QWidget;
        page2->setLayout(layout);
        tabs->addTab(page2, "Search");

        QVBoxLayout* main_layout = new QVBoxLayout();
        main_layout->addWidget(tabs);
        main_layout->addLayout(button_layout);
        setLayout(main_layout);

        setWindowTitle(tr("Preferences"));
    }

    void
    PreferencesDialog::write_back(void) {
        hide_failed = hide_check->isChecked();
        zoom = zoom_check->isChecked();
        refresh = refresh_box->value();
        refresh_pause = slow_box->isChecked() ? 200 : 0;
        move_during_search = move_during_search_box->isChecked();
        smooth_scroll_and_zoom = smooth_check->isChecked();
        copies = copies_check->isChecked();
        c_d = cd_box->value();
        a_d = ad_box->value();
        QSettings settings("gecode.dev", "Gist");
        settings.setValue("search/hideFailed", hide_failed);
        settings.setValue("search/zoom", zoom);
        settings.setValue("search/copies", copies);
        settings.setValue("search/refresh", refresh);
        settings.setValue("search/refreshPause", refresh_pause);
        settings.setValue("smoothScrollAndZoom", smooth_scroll_and_zoom);

        accept();
    }

    void
    PreferencesDialog::defaults(void) {
        hide_failed = true;
        zoom = false;
        refresh = 500;
        refresh_pause = 0;
        smooth_scroll_and_zoom = true;
        move_during_search = false;
        copies = false;
        c_d = 8;
        a_d = 2;
        hide_check->setChecked(hide_failed);
        zoom_check->setChecked(zoom);
        refresh_box->setValue(refresh);
        slow_box->setChecked(refresh_pause > 0);
        smooth_check->setChecked(smooth_scroll_and_zoom);
        copies_check->setChecked(copies);
        move_during_search_box->setChecked(move_during_search);
    }

    void
    PreferencesDialog::toggle_slow(int state) {
        refresh_box->setEnabled(state != Qt::Checked);
    }

}} // namespace Gecode::Gist

// STATISTICS: gist-any
