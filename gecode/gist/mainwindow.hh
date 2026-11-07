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

#ifndef GECODE_GIST_MAINWINDOW_HH
#define GECODE_GIST_MAINWINDOW_HH

#include <gecode/gist.hh>
#include <gecode/gist/qtgist.hh>

namespace Gecode::Gist {

    /// Display information about %Gist
    class AboutGist : public QDialog {
    public:
        /// Constructor
        AboutGist(QWidget* parent = nullptr);
    };

    /**
     * \brief Main window for stand-alone %Gist
     *
     * \ingroup TaskGist
     */
    class GECODE_GIST_EXPORT GistMainWindow : public QMainWindow {
        Q_OBJECT
    private:
        /// Options
        Options opt;
        /// Whether search is currently running
        bool is_searching;
        /// Status bar label for maximum depth indicator
        QLabel* depth_label;
        /// Status bar label for number of solutions
        QLabel* solved_label;
        /// Status bar label for number of failures
        QLabel* failed_label;
        /// Status bar label for number of choices
        QLabel* choices_label;
        /// Status bar label for number of open nodes
        QLabel* open_label;
        /// Menu for solution inspectors
        QMenu* solution_inspectors_menu;
        /// Menu for double click inspectors
        QMenu* double_click_inspectors_menu;
        /// Menu for move inspectors
        QMenu* move_inspectors_menu;
        /// Menu for comparators
        QMenu* comparators_menu;
        /// Menu for bookmarks
        QMenu* bookmarks_menu;
        /// Menu for direct node inspection
        QMenu* inspect_node_menu;
        /// Menu for direct node inspection before fixpoint
        QMenu* inspect_node_before_fp_menu;
        /// Action for activating the preferences menu
        QAction* pref_action;

    protected:
        /// The contained %Gist object
        Gist* c;
        /// A menu bar
        QMenuBar* menu_bar;
        /// About dialog
        AboutGist about_gist;

        /// The status has changed (e.g., new solutions have been found)
        void status_changed(const Statistics& stats, bool finished);
        /// Open the about dialog
        void about();
        /// Open the preferences dialog
        void preferences(bool setup = false);
        /// Populate the inspector menus from the actions found in Gist
        void populate_inspector_selection();
        /// Populate the inspector menus from the actions found in Gist
        void populate_inspectors();
        /// Populate the bookmarks menus from the actions found in Gist
        void populate_bookmarks();

    public:
        /// Constructor
        GistMainWindow(Space* root, bool bab, const Options& opt);

    protected:
        /// Close Gist
        void closeEvent(QCloseEvent* event) override;
    };

} // namespace Gecode::Gist

#endif

// STATISTICS: gist-any
