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

#ifndef GECODE_GIST_QTGIST_HH
#define GECODE_GIST_QTGIST_HH

#include <gecode/gist/treecanvas.hh>
#include <gecode/gist/nodestats.hh>

/*
 * Configure linking
 *
 */

#if !defined(GECODE_STATIC_LIBS) && !defined(GIST_STATIC_LIBS) &&                                                      \
    (defined(__CYGWIN__) || defined(__MINGW32__) || defined(_MSC_VER))

#ifdef GECODE_BUILD_GIST
#define GECODE_GIST_EXPORT __declspec(dllexport)
#else
#define GECODE_GIST_EXPORT __declspec(dllimport)
#endif

#else

#ifdef GECODE_GCC_HAS_CLASS_VISIBILITY
#define GECODE_GIST_EXPORT __attribute__((visibility("default")))
#else
#define GECODE_GIST_EXPORT
#endif

#endif

namespace Gecode { namespace Gist {

    /**
     * \brief %Gecode Interactive %Search Tool
     *
     * This class provides an interactive search tree viewer and explorer as
     * a Qt widget. You can embed or inherit from this widget to use %Gist
     * in your own project.
     *
     * \ingroup TaskGist
     */
    class GECODE_GIST_EXPORT Gist : public QWidget {
        Q_OBJECT
    private:
        /// The canvas implementation
        TreeCanvas* canvas;
        /// The time slider
        QSlider* time_bar;
        /// Context menu
        QMenu* context_menu;
        /// Action used when no solution inspector is registered
        QAction* null_solution_inspector;
        /// Menu of solution inspectors
        QMenu* solution_inspector_menu;
        /// Action used when no double click inspector is registered
        QAction* null_double_click_inspector;
        /// Menu of double click inspectors
        QMenu* double_click_inspector_menu;
        /// Action used when no double click inspector is registered
        QAction* null_move_inspector;
        /// Menu of double click inspectors
        QMenu* move_inspector_menu;
        /// Action used when no comparator is registered
        QAction* null_comparator;
        /// Menu of comparators
        QMenu* comparator_menu;
        /// Action used when no bookmark exists
        QAction* null_bookmark;
        /// Bookmark menu
        QMenu* bookmarks_menu;
        /// Menu for direct node inspection
        QMenu* inspect_node_menu;
        /// Menu for direct node inspection before fixpoint
        QMenu* inspect_node_before_fp_menu;
        /// Information about individual nodes
        NodeStatInspector* node_stat_inspector;

    public:
        /// Inspect current node
        QAction* inspect;
        /// Inspect current node before fixpoint
        QAction* inspect_before_fp;
        /// Stop search
        QAction* stop;
        /// Reset %Gist
        QAction* reset;
        /// Navigate to parent node
        QAction* nav_up;
        /// Navigate to leftmost child node
        QAction* nav_down;
        /// Navigate to left sibling
        QAction* nav_left;
        /// Navigate to right sibling
        QAction* nav_right;
        /// Navigate to root node
        QAction* nav_root;
        /// Navigate to next solution (to the left)
        QAction* nav_next_sol;
        /// Navigate to previous solution (to the right)
        QAction* nav_prev_sol;
        /// Search next solution in current subtree
        QAction* search_next;
        /// Search all solutions in current subtree
        QAction* search_all;
        /// Toggle whether current node is hidden
        QAction* toggle_hidden;
        /// Hide failed subtrees under current node
        QAction* hide_failed;
        /// Unhide all hidden subtrees under current node
        QAction* unhide_all;
        /// Label branches under current node
        QAction* label_branches;
        /// Label branches on path to root
        QAction* label_path;
        /// Zoom tree to fit window
        QAction* zoom_to_fit;
        /// Center on current node
        QAction* center;
        /// Export PDF of current subtree
        QAction* export_pdf;
        /// Export PDF of whole tree
        QAction* export_whole_tree_pdf;
        /// Print tree
        QAction* print;

        /// Bookmark current node
        QAction* bookmark_node;
        /// Compare current node to other node
        QAction* compare_node;
        /// Compare current node to other node before fixpoint
        QAction* compare_node_before_fp;
        /// Set path from current node to the root
        QAction* set_path;
        /// Inspect all nodes on selected path
        QAction* inspect_path;
        /// Open node statistics inspector
        QAction* show_node_stats;
        /// Bookmark current node
        QAction* toggle_stop;
        /// Bookmark current node
        QAction* unstop_all;

        /// Group of all actions for solution inspectors
        QActionGroup* solution_inspector_group;
        /// Group of all actions for double click inspectors
        QActionGroup* double_click_inspector_group;
        /// Group of all actions for move inspectors
        QActionGroup* move_inspector_group;
        /// Group of all actions for comparators
        QActionGroup* comparator_group;
        /// Group of all actions for bookmarks
        QActionGroup* bookmarks_group;
        /// Group of all actions for direct inspector selection
        QActionGroup* inspect_group;
        /// Group of all actions for direct inspector selection
        QActionGroup* inspect_before_fp_group;

        /// Constructor
        Gist(Space* root, bool bab, QWidget* parent, const Options& opt);
        /// Destructor
        ~Gist() override;

        /// Add double click inspector \a ins
        void add_double_click_inspector(Inspector* ins);
        /// Add solution inspector \a ins
        void add_solution_inspector(Inspector* ins);
        /// Add move inspector \a ins
        void add_move_inspector(Inspector* ins);
        /// Add comparator \a c0
        void add_comparator(Comparator* c0);

        /// Set preference whether to automatically hide failed subtrees
        void set_auto_hide_failed(bool b);
        /// Set preference whether to automatically zoom to fit
        void set_auto_zoom(bool b);
        /// Return preference whether to automatically hide failed subtrees
        bool get_auto_hide_failed();
        /// Return preference whether to automatically zoom to fit
        bool get_auto_zoom();
        /// Set preference whether to show copies in the tree
        void set_show_copies(bool b);
        /// Return preference whether to show copies in the tree
        bool get_show_copies();

        /// Set refresh rate
        void set_refresh(int i);
        /// Set refresh pause in msec
        void set_refresh_pause(int i);
        /// Return preference whether to use smooth scrolling and zooming
        bool get_smooth_scroll_and_zoom();
        /// Set preference whether to use smooth scrolling and zooming
        void set_smooth_scroll_and_zoom(bool b);
        /// Return preference whether to move cursor during search
        bool get_move_during_search();
        /// Set preference whether to move cursor during search
        void set_move_during_search(bool b);

        /// Set recomputation parameters \a c_d and \a a_d
        void set_recomp_distances(int c_d, int a_d);
        /// Return recomputation distance
        int get_cd();
        /// Return adaptive recomputation distance
        int get_ad();

        /// Stop search and wait until finished
        bool finish();

    protected:
        /// Handle resize event
        void resizeEvent(QResizeEvent*) override;

    Q_SIGNALS:
        /// Signals that the tree has changed
        void status_changed(const Statistics&, bool);

        /// Signals that a solution has been found
        void solution(const Space*);

        /// Signals that %Gist is ready to be closed
        void search_finished();

    private Q_SLOTS:
        /// Displays the context menu for a node
        void on_canvas_context_menu(QContextMenuEvent*);
        /// Reacts on status changes
        void on_canvas_status_changed(VisualNode*, const Statistics&, bool);
        /// Reacts on double click inspector selection
        void select_double_click_inspector(QAction*);
        /// Reacts on solution inspector selection
        void select_solution_inspector(QAction*);
        /// Reacts on move inspector selection
        void select_move_inspector(QAction*);
        /// Reacts on comparator selection
        void select_comparator(QAction*);
        /// Reacts on bookmark selection
        void select_bookmark(QAction*);
        /// Reacts on adding a bookmark
        void add_bookmark(const QString& id);
        /// Reacts on removing a bookmark
        void remove_bookmark(int idx);
        /// Populate the inspector menus from the actions found in Gist
        void populate_inspectors();
        /// Populate the bookmarks menu
        void populate_bookmarks_menu();
        /// Shows node status information
        void show_stats();
        /// Inspect current node with inspector described by \a a
        void inspect_with_action(QAction* a);
        /// Inspect current node with inspector described by \a a
        void inspect_before_fp_with_action(QAction* a);

    protected:
        /// Add inspector \a i0
        void add_inspector(Inspector* i, QAction*& nas, QAction*& nad, QAction*& nam);
    };

}} // namespace Gecode::Gist

#endif

// STATISTICS: gist-any
