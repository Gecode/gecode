/*
 *  Main authors:
 *     Maxim Shishmarev <maxim.shishmarev@monash.edu>
 *
 *  Contributing authors:
 *     Guido Tack <tack@gecode.dev>
 *     Kevin Leo <kevin.leo@monash.edu>
 *
 *  Copyright:
 *     Maxim Shishmarev, 2017
 *     Guido Tack, 2017
 *     Kevin Leo, 2017
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.dev
 *
 *  Permission is hereby granted, free of charge, to any person obtaining
 *  a copy of this software and associated documentation files (the
 *  "Software"), to deal in the Software without restriction, including
 *  without limitation the rights to use, copy, modify, merge, publish,
 *  distribute, sublicense, and/or sell copies of the Software, and to
 *  permit persons to whom the Software is furnished to do so, subject to
 *  the following conditions:
 *
 *  The above copyright notice and this permission notice shall be
 *  included in all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 *  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 *  NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
 *  LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
 *  OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 *  WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 */

#include <vector>
#include <string>
#include <cassert>
#include <cstdint>

namespace Gecode { namespace CPProfiler {

    /// Version of CPProfiler protocol
    static const int32_t PROFILER_PROTOCOL_VERSION = 3;

    /// Types of nodes for CP Profiler
    enum class NodeStatus {
        SOLVED = 0, ///< Node representing a solution
        FAILED = 1, ///< Node representing failure
        BRANCH = 2, ///< Node representing a branch
        SKIPPED = 3 ///< Node skipped by backjumping
    };
    using enum NodeStatus;

    /// Types of messages for CP Profiler
    enum class MsgType { NODE = 0, DONE = 1, START = 2, RESTART = 3 };

    /// Optional value class
    template <class T>
    class Option {
    protected:
        /// A value, potentially not initialized
        T value_;
        /// Whether value is present
        bool present{false};

    public:
        /// Check whether value is present
        bool valid() const;
        /// Set value to \a t
        void set(const T& t);
        /// Disregard value
        void unset();
        /// Access value
        const T& value() const;
        /// Access value
        T& value();
    };

    template <class T>
    forceinline bool
    Option<T>::valid() const {
        return present;
    }
    template <class T>
    forceinline void
    Option<T>::set(const T& t) {
        present = true;
        value_ = t;
    }
    template <class T>
    forceinline void
    Option<T>::unset() {
        present = false;
    }
    template <class T>
    forceinline const T&
    Option<T>::value() const {
        assert(present);
        return value_;
    }
    template <class T>
    forceinline T&
    Option<T>::value() {
        assert(present);
        return value_;
    }

    /// Unique identifier for a node
    struct NodeUID {
        /// Node number
        int32_t nid;
        /// Restart id
        int32_t rid;
        /// Thread id
        int32_t tid;
    };

    /// Message for the CP Profiler
    class Message {
    protected:
        MsgType _type;

        NodeUID _node;
        NodeUID _parent;
        int32_t _alt;
        int32_t _kids;
        NodeStatus _status;

        bool _have_label{false};
        std::string _label;

        bool _have_nogood{false};
        std::string _nogood;

        bool _have_info{false};
        std::string _info;

        bool _have_version{false};
        int32_t _version; // PROFILER_PROTOCOL_VERSION;

    public:
        bool
        is_node() const {
            return _type == MsgType::NODE;
        }
        bool
        is_done() const {
            return _type == MsgType::DONE;
        }
        bool
        is_start() const {
            return _type == MsgType::START;
        }
        bool
        is_restart() const {
            return _type == MsgType::RESTART;
        }

        NodeUID
        node_uid() const {
            return _node;
        }
        void
        set_node_uid(const NodeUID& n) {
            _node = n;
        }

        NodeUID
        parent_uid() const {
            return _parent;
        }
        void
        set_parent_uid(const NodeUID& p) {
            _parent = p;
        }

        int32_t
        alt() const {
            return _alt;
        }
        void
        set_alt(int32_t alt) {
            _alt = alt;
        }

        int32_t
        kids() const {
            return _kids;
        }
        void
        set_kids(int32_t kids) {
            _kids = kids;
        }

        NodeStatus
        status() const {
            return _status;
        }
        void
        set_status(NodeStatus status) {
            _status = status;
        }

        void
        set_label(const std::string& label) {
            _have_label = true;
            _label = label;
        }

        void
        set_info(const std::string& info) {
            _have_info = true;
            _info = info;
        }

        void
        set_nogood(const std::string& nogood) {
            _have_nogood = true;
            _nogood = nogood;
        }

        void
        set_version(int32_t v) {
            _have_version = true;
            _version = v;
        }

        bool
        has_version() const {
            return _have_version;
        }
        int32_t
        version() const {
            return _version;
        }

        bool
        has_label() const {
            return _have_label;
        }
        const std::string&
        label() const {
            return _label;
        }

        bool
        has_nogood() const {
            return _have_nogood;
        }
        const std::string&
        nogood() const {
            return _nogood;
        }

        // generic optional fields
        bool
        has_info() const {
            return _have_info;
        }
        const std::string&
        info() const {
            return _info;
        }

        void
        set_type(MsgType type) {
            _type = type;
        }
        MsgType
        type() const {
            return _type;
        }

        void
        reset() {
            _have_label = false;
            _have_nogood = false;
            _have_info = false;
            _have_version = false;
        }
    };

    class MessageMarshalling {
    private:
        /// Only optional fields are listed here, if node (no need for field id)
        enum class Field { LABEL = 0, NOGOOD = 1, INFO = 2, VERSION = 3 };
        using enum Field;

        Message msg;

        using iter = char*;

        static void
        serialize_type(std::vector<char>& data, MsgType f) {
            data.push_back(static_cast<char>(f));
        }

        static void
        serialize_field(std::vector<char>& data, Field f) {
            data.push_back(static_cast<char>(f));
        }

        static void
        serialize(std::vector<char>& data, int32_t i) {
            data.push_back(static_cast<char>((i & 0xFF000000) >> 24));
            data.push_back(static_cast<char>((i & 0xFF0000) >> 16));
            data.push_back(static_cast<char>((i & 0xFF00) >> 8));
            data.push_back(static_cast<char>((i & 0xFF)));
        }

        static void
        serialize(std::vector<char>& data, NodeStatus s) {
            data.push_back(static_cast<char>(s));
        }

        static void
        serialize(std::vector<char>& data, const std::string& s) {
            serialize(data, static_cast<int32_t>(s.size()));
            for (char c : s) {
                data.push_back(c);
            }
        }

        static MsgType
        deserialize_msg_type(iter& it) {
            auto m = static_cast<MsgType>(*it);
            ++it;
            return m;
        }

        static Field
        deserialize_field(iter& it) {
            auto f = static_cast<Field>(*it);
            ++it;
            return f;
        }

        static int32_t
        deserialize_int(iter& it) {
            auto b1 = static_cast<uint32_t>(reinterpret_cast<uint8_t&>(*it++));
            auto b2 = static_cast<uint32_t>(reinterpret_cast<uint8_t&>(*it++));
            auto b3 = static_cast<uint32_t>(reinterpret_cast<uint8_t&>(*it++));
            auto b4 = static_cast<uint32_t>(reinterpret_cast<uint8_t&>(*it++));

            return static_cast<int32_t>(b1 << 24 | b2 << 16 | b3 << 8 | b4);
        }

        static NodeStatus
        deserialize_status(iter& it) {
            auto f = static_cast<NodeStatus>(*it);
            ++it;
            return f;
        }

        static std::string
        deserialize_string(iter& it) {
            std::string result;
            int32_t size = deserialize_int(it);
            result.reserve(static_cast<size_t>(size));
            for (int32_t i = 0; i < size; i++) {
                result += *it;
                ++it;
            }
            return result;
        }

    public:
        Message&
        make_node(NodeUID node, NodeUID parent, int32_t alt, int32_t kids, NodeStatus status) {
            msg.reset();
            msg.set_type(MsgType::NODE);

            msg.set_node_uid(node);
            msg.set_parent_uid(parent);

            msg.set_alt(alt);
            msg.set_kids(kids);
            msg.set_status(status);

            return msg;
        }

        void
        make_start(const std::string& info) {
            msg.reset();
            msg.set_type(MsgType::START);
            msg.set_version(PROFILER_PROTOCOL_VERSION);
            msg.set_info(info); /// info contains name, has_restarts, execution id
        }

        void
        make_restart(const std::string& info) {
            msg.reset();
            msg.set_type(MsgType::RESTART);
            msg.set_info(info); /// info contains restart_id (-1 default)
        }

        void
        make_done() {
            msg.reset();
            msg.set_type(MsgType::DONE);
        }

        const Message&
        get_msg() {
            return msg;
        }

        std::vector<char>
        serialize() const {
            std::vector<char> data;
            size_t data_size =
                1 + (msg.is_node() ? 4 * 8 + 1 : 0) + (msg.has_label() ? 1 + 4 + msg.label().size() : 0) +
                (msg.has_nogood() ? 1 + 4 + msg.nogood().size() : 0) + (msg.has_info() ? 1 + 4 + msg.info().size() : 0);
            data.reserve(data_size);

            serialize_type(data, msg.type());
            if (msg.is_node()) {
                // serialize NodeId node
                auto n_uid = msg.node_uid();
                serialize(data, n_uid.nid);
                serialize(data, n_uid.rid);
                serialize(data, n_uid.tid);
                // serialize NodeId parent
                auto p_uid = msg.parent_uid();
                serialize(data, p_uid.nid);
                serialize(data, p_uid.rid);
                serialize(data, p_uid.tid);
                // Other Data
                serialize(data, msg.alt());
                serialize(data, msg.kids());
                serialize(data, msg.status());
            }

            if (msg.has_version()) {
                serialize_field(data, VERSION);
                serialize(data, msg.version());
            }
            if (msg.has_label()) {
                serialize_field(data, LABEL);
                serialize(data, msg.label());
            }
            if (msg.has_nogood()) {
                serialize_field(data, NOGOOD);
                serialize(data, msg.nogood());
            }
            if (msg.has_info()) {
                serialize_field(data, INFO);
                serialize(data, msg.info());
            }
            return data;
        }

        void
        deserialize(char* data, size_t size) {
            char* end = data + size;
            msg.set_type(deserialize_msg_type(data));
            if (msg.is_node()) {
                int32_t nid = deserialize_int(data);
                int32_t rid = deserialize_int(data);
                int32_t tid = deserialize_int(data);

                msg.set_node_uid({nid, rid, tid});

                nid = deserialize_int(data);
                rid = deserialize_int(data);
                tid = deserialize_int(data);

                msg.set_parent_uid({nid, rid, tid});

                msg.set_alt(deserialize_int(data));
                msg.set_kids(deserialize_int(data));
                msg.set_status(deserialize_status(data));
            }

            msg.reset();

            while (data != end) {
                MessageMarshalling::Field f = deserialize_field(data);
                switch (f) {
                case VERSION:
                    msg.set_version(deserialize_int(data));
                    break;
                case LABEL:
                    msg.set_label(deserialize_string(data));
                    break;
                case NOGOOD:
                    msg.set_nogood(deserialize_string(data));
                    break;
                case INFO:
                    msg.set_info(deserialize_string(data));
                    break;
                default:
                    break;
                }
            }
        }
    };

}} // namespace Gecode::CPProfiler

// STATISTICS: search-trace
