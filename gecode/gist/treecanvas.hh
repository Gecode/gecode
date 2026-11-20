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

#ifndef GECODE_GIST_TREECANVAS_HH
#define GECODE_GIST_TREECANVAS_HH

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif

#include <gecode/kernel.hh>
#include <gecode/gist.hh>

#include <gecode/gist/visualnode.hh>

#if QT_VERSION >= 0x060000
using GecodeQMutex = QRecursiveMutex;
#else
using GecodeQMutex = QMutex;
#endif

namespace Gecode::Gist {

    /// \brief Parameters for the tree layout
    namespace LayoutConfig {
        /// Minimum scale factor
        const int min_scale = 10;
        /// Maximum scale factor
        const int max_scale = 400;
        /// Default scale factor
        const int def_scale = 100;
        /// Maximum scale factor for automatic zoom
        const int max_auto_zoom_scale = def_scale;
    } // namespace LayoutConfig

    class TreeCanvas;

    /// \brief A thread that concurrently explores the tree
    class SearcherThread : public QThread {
        Q_OBJECT
    private:
        VisualNode* node;
        int depth;
        bool a;
        TreeCanvas* t;
        void update_canvas();

    public:
        void search(VisualNode* n, bool all, TreeCanvas* ti);

    Q_SIGNALS:
        void update(int w, int h, int scale0);
        void status_changed(bool);
        void scale_changed(int);
        void solution(const Space*);
        void search_finished();
        void move_to_node(VisualNode* n, bool);

    protected:
        void run() override;
    };

    /// \brief A canvas that displays the search tree
    class GECODE_GIST_EXPORT TreeCanvas : public QWidget {
        Q_OBJECT

        friend class SearcherThread;
        friend class Gist;

    public:
        /// Constructor
        TreeCanvas(Space* root_space, bool bab, QWidget* parent, const Options& opt);
        /// Destructor
        ~TreeCanvas() override;

        /// Add inspector \a i
        void add_double_click_inspector(Inspector* i);
        /// Set active inspector
        void activate_double_click_inspector(int i, bool active);
        /// Add inspector \a i
        void add_solution_inspector(Inspector* i);
        /// Set active inspector
        void activate_solution_inspector(int i, bool active);
        /// Add inspector \a i
        void add_move_inspector(Inspector* i);
        /// Set active inspector
        void activate_move_inspector(int i, bool active);
        /// Add comparator \a c
        void add_comparator(Comparator* c);
        /// Set active comparator
        void activate_comparator(int i, bool active);

        /// Set scale factor to \a scale0
        void scale_tree(int scale0, int zoomx = -1, int zoomy = -1);

        /// Explore complete subtree of selected node
        void search_all();
        /// Find next solution below selected node
        void search_one();
        /// Toggle hidden state of selected node
        void toggle_hidden();
        /// Hide failed subtrees of selected node
        void hide_failed();
        /// Unhide all nodes below selected node
        void unhide_all();
        /// Do not stop at selected stop node
        void toggle_stop();
        /// Do not stop at any stop node
        void unstop_all();
        /// Export pdf of the current subtree
        void export_pdf();
        /// Export pdf of the whole tree
        void export_whole_tree_pdf();
        /// Print the tree
        void print();
        /// Zoom the canvas so that the whole tree fits
        void zoom_to_fit();
        /// Center the view on the currently selected node
        void center_current_node();
        /**
         * \brief Call the double click inspector for the currently selected node
         *
         * If \a fix is true, then the node is inspected after fixpoint
         * computation, otherwise its status after branching but before
         * fixpoint computation is inspected.
         */
        void inspect_current_node(bool fix = true, int inspector_no = -1);
        /// Calls inspect_current_node(false)
        void inspect_before_fp();
        /// Label all branches in subtree under current node
        void label_branches();
        /// Label all branches on path to root node
        void label_path();

        /// Stop current search
        void stop_search();

        /// Reset
        void reset();

        /// Move selection to the parent of the selected node
        void nav_up();
        /// Move selection to the first child of the selected node
        void nav_down();
        /// Move selection to the left sibling of the selected node
        void nav_left();
        /// Move selection to the right sibling of the selected node
        void nav_right();
        /// Move selection to the root node
        void nav_root();
        /// Move selection to next solution (in DFS order)
        void nav_next_sol(bool back = false);
        /// Move selection to previous solution (in DFS order)
        void nav_prev_sol();

        /// Bookmark current node
        void bookmark_node();
        /// Set the current node to be the head of the path
        void set_path();
        /// Call the double click inspector for all nodes on the path from root to head of the path
        void inspect_path();
        /// Wait for click on node to compare with current node
        void start_compare_nodes();
        /// Wait for click on node to compare with current node before fixpoint
        void start_compare_nodes_before_fp();

        /// Re-emit status change information for current node
        void emit_status_changed();

        /// Set recomputation distances
        void set_recomp_distances(int c_d, int a_d);
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
        /// Resize to the outer widget size if auto zoom is enabled
        void resize_to_outer();

        /// Stop search and wait for it to finish
        bool finish();

    Q_SIGNALS:
        /// The scale factor has changed
        void scale_changed(int);
        /// The auto-zoom state was changed
        void auto_zoom_changed(bool);
        /// Context menu triggered
        void context_menu(QContextMenuEvent*);
        /// Status bar update
        void status_changed(VisualNode*, const Statistics&, bool);
        /// Signals that a solution has been found
        void solution(const Space*);
        /// Signals that %Gist is finished
        void search_finished();
        /// Signals that a bookmark has been added
        void added_bookmark(const QString& id);
        /// Signals that a bookmark has been removed
        void removed_bookmark(int idx);

    protected:
        /// Mutex for synchronizing access to the tree
        GecodeQMutex mutex;
        /// Mutex for synchronizing layout and drawing
        GecodeQMutex layout_mutex;
        /// Search engine thread
        SearcherThread searcher;
        /// Flag signalling the search to stop
        bool stop_search_flag;
        /// Flag signalling that Gist is ready to be closed
        bool finished_flag;
        /// Allocator for nodes
        Node::NodeAllocator* na;
        /// The root node of the tree
        VisualNode* root;
        /// The currently best solution (for branch-and-bound)
        BestNode* cur_best;
        /// The currently selected node
        VisualNode* current_node;
        /// The head of the currently selected path
        VisualNode* path_head;
        /// The registered click inspectors, and whether they are active
        QVector<QPair<Inspector*, bool>> double_click_inspectors;
        /// The registered solution inspectors, and whether they are active
        QVector<QPair<Inspector*, bool>> solution_inspectors;
        /// The registered move inspectors, and whether they are active
        QVector<QPair<Inspector*, bool>> move_inspectors;
        /// The registered comparators, and whether they are active
        QVector<QPair<Comparator*, bool>> comparators;

        /// The bookmarks map
        QVector<VisualNode*> bookmarks;

        /// Whether node comparison action is running
        bool compare_nodes;
        /// Whether node comparison action computes fixpoint
        bool compare_nodes_before_fp;

        /// The scale bar
        QSlider* scale_bar;

        /// Statistics about the search tree
        Statistics stats;

        /// Current scale factor
        double scale;
        /// Offset on the x axis so that the tree is centered
        int xtrans;

        /// Whether to hide failed subtrees automatically
        bool auto_hide_failed;
        /// Whether to zoom automatically
        bool auto_zoom;
        /// Whether to show copies in the tree
        bool show_copies;
        /// Refresh rate
        int refresh;
        /// Time (in msec) to pause after each refresh
        int refresh_pause;
        /// Whether to use smooth scrolling and zooming
        bool smooth_scroll_and_zoom;
        /// Whether to move cursor during search
        bool move_during_search;

        /// The recomputation distance
        int c_d;
        /// The adaptive recomputation distance
        int a_d;

        /// Return the node corresponding to the \a event position
        VisualNode* event_node(QEvent* event);
        /// General event handler, used for displaying tool tips
        bool event(QEvent* event) override;
        /// Paint the tree
        void paintEvent(QPaintEvent* event) override;
        /// Handle mouse press event
        void mousePressEvent(QMouseEvent* event) override;
        /// Handle mouse double click event
        void mouseDoubleClickEvent(QMouseEvent* event) override;
        /// Handle context menu event
        void contextMenuEvent(QContextMenuEvent* event) override;
        /// Handle resize event
        void resizeEvent(QResizeEvent* event) override;
        /// Handle mouse wheel events
        void wheelEvent(QWheelEvent* event) override;

        /// Timer for smooth zooming
        QTimeLine zoom_time_line;
        /// Timer for smooth scrolling
        QTimeLine scroll_time_line;
        /// Target x coordinate after smooth scrolling
        int target_x;
        /// Source x coordinate after smooth scrolling
        int source_x;
        /// Target y coordinate after smooth scrolling
        int target_y;
        /// Target y coordinate after smooth scrolling
        int source_y;

        /// Target width after layout
        int target_w;
        /// Target height after layout
        int target_h;
        /// Target scale after layout
        int target_scale;
        /// Timer id for delaying the update
        int layout_done_timer_id;

        /// Timer invoked for smooth zooming and scrolling
        void timerEvent(QTimerEvent* e) override;

    public Q_SLOTS:
        /// Update display
        void update();
        /// React to scroll events
        void scroll();
        /// Layout done
        void layout_done(int w, int h, int scale0);
        /// Set the selected node to \a n
        void set_current_node(VisualNode* n, bool finished = true, bool update = true);
    private Q_SLOTS:
        /// Search has finished
        void status_changed(bool);
        /// Export PDF of the subtree of \a n
        void export_node_pdf(VisualNode* n);
        /// A new solution \a s has been found
        void inspect_solution(const Space* s);
        /// Scroll to \a i percent of the target
        void scroll(int i);
    };

} // namespace Gecode::Gist

#endif

// STATISTICS: gist-any
