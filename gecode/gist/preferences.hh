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

#ifndef GECODE_GIST_PREFERENCES_HH
#define GECODE_GIST_PREFERENCES_HH

#include <QtGui>
#if QT_VERSION >= 0x050000
#include <QtWidgets>
#endif
#include <gecode/gist.hh>

namespace Gecode::Gist {

    /**
     * \brief Preferences dialog for %Gist
     */
    class PreferencesDialog : public QDialog {
        Q_OBJECT

    protected:
        QCheckBox* hide_check;
        QCheckBox* zoom_check;
        QCheckBox* smooth_check;
        QCheckBox* copies_check;
        QSpinBox* refresh_box;
        QCheckBox* slow_box;
        QCheckBox* move_during_search_box;
        QSpinBox* cd_box;
        QSpinBox* ad_box;

        /// Write settings
        void write_back();
        /// Reset to defaults
        void defaults();
        /// Toggle slow down setting
        void toggle_slow(int state);

    public:
        /// Constructor
        PreferencesDialog(const Options& opt, QWidget* parent = nullptr);

        /// Whether to automatically hide failed subtrees during search
        bool hide_failed;
        /// Whether to automatically zoom during search
        bool zoom;
        /// Whether to show where copies are in the tree
        bool copies;
        /// How often to refresh the display during search
        int refresh;
        /// Milliseconds to wait after each refresh (to slow down search)
        int refresh_pause;
        /// Whether to use smooth scrolling and zooming
        bool smooth_scroll_and_zoom;
        /// Whether to move cursor during search
        bool move_during_search;

        /// The copying distance
        int c_d;
        /// The adaptive recomputation distance
        int a_d;
    };

} // namespace Gecode::Gist

#endif

// STATISTICS: gist-any
