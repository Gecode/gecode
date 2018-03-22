/* -*- mode: C++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
/*
 *  Main authors:
 *     Maxim Shishmarev <maxim.shishmarev@monash.edu>
 *
 *  Contributing authors:
 *     Guido Tack <tack@gecode.org>
 *     Kevin Leo <kevin.leo@monash.edu>
 *
 *  Copyright:
 *     Maxim Shishmarev, 2017
 *     Guido Tack, 2017
 *     Kevin Leo, 2017
 *
 *  This file is part of Gecode, the generic constraint
 *  development environment:
 *     http://www.gecode.org
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
  enum NodeStatus {
    SOLVED  = 0, ///< Node representing a solution
    FAILED  = 1, ///< Node representing failure
    BRANCH  = 2, ///< Node representing a branch
    SKIPPED = 3  ///< Node skipped by backjumping
  };
  
  /// Types of messages for CP Profiler
  enum class MsgType {
    NODE    = 0,
    DONE    = 1,
    START   = 2,
    RESTART = 3
  };
  
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
    bool valid(void) const;
    /// Set value to \a t
    void set(const T& t);
    /// Disregard value
    void unset(void);
    /// Access value
    const T& value(void) const;
    /// Access value
    T& value(void);
  };

  template <class T>
  forceinline bool
  Option<T>::valid(void) const {
    return present;
  }
  template <class T>
  forceinline void
  Option<T>::set(const T& t) {
    present = true; value_ = t;
  }
  template <class T>
  forceinline void
  Option<T>::unset(void) {
    present = false;
  }
  template <class T>
  forceinline const T&
  Option<T>::value(void) const {
    assert(present); return value_;
  }
  template <class T>
  forceinline T&
  Option<T>::value(void) {
    assert(present); return value_;
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
    bool isNode(void) const { return _type == MsgType::NODE; }
    bool isDone(void) const { return _type == MsgType::DONE; }
    bool isStart(void) const { return _type == MsgType::START; }
    bool isRestart(void) const { return _type == MsgType::RESTART; }
    
    NodeUID nodeUID(void) const { return _node; }
    void set_nodeUID(const NodeUID& n) { _node = n; }
    
    NodeUID parentUID(void) const { return _parent; }
    void set_parentUID(const NodeUID& p) { _parent = p; }
    
    int32_t alt(void) const { return _alt; }
    void set_alt(int32_t alt) { _alt = alt; }
    
    int32_t kids(void) const { return _kids; }
    void set_kids(int32_t kids) { _kids = kids; }
    
    NodeStatus status(void) const { return _status; }
    void set_status(NodeStatus status) { _status = status; }
    
    void set_label(const std::string& label) {
      _have_label = true;
      _label = label;
    }
    
    void set_info(const std::string& info) {
      _have_info = true;
      _info = info;
    }
    
    void set_nogood(const std::string& nogood) {
      _have_nogood = true;
      _nogood = nogood;
    }
    
    void set_version(int32_t v) {
      _have_version = true;
      _version = v;
    }
    
    bool has_version(void) const { return _have_version; }
    int32_t version(void) const { return _version; }
    
    bool has_label(void) const { return _have_label; }
    const std::string& label() const { return _label; }
    
    bool has_nogood(void) const { return _have_nogood; }
    const std::string& nogood(void) const { return _nogood; }
    
    // generic optional fields
    bool has_info(void) const { return _have_info; }
    const std::string& info(void) const { return _info; }

    void set_type(MsgType type) { _type = type; }
    MsgType type(void) const { return _type; }
    
    void reset(void) {
      _have_label = false;
      _have_nogood = false;
      _have_info = false;
      _have_version = false;
    }
  };


  class MessageMarshalling {
  private:
    /// Only optional fields are listed here, if node (no need for field id)
    enum Field {
      LABEL = 0,
      NOGOOD = 1,
      INFO = 2,
      VERSION = 3
    };
    
    Message msg;
    
    typedef char* iter;
    
    static void serializeType(std::vector<char>& data, MsgType f) {
      data.push_back(static_cast<char>(f));
    }
    
    static void serializeField(std::vector<char>& data, Field f) {
      data.push_back(static_cast<char>(f));
    }
    
    static void serialize(std::vector<char>& data, int32_t i) {
      data.push_back(static_cast<char>((i & 0xFF000000) >> 24));
      data.push_back(static_cast<char>((i & 0xFF0000) >> 16));
      data.push_back(static_cast<char>((i & 0xFF00) >> 8));
      data.push_back(static_cast<char>((i & 0xFF)));
    }
    
    static void serialize(std::vector<char>& data, NodeStatus s) {
      data.push_back(static_cast<char>(s));
    }
    
    static void serialize(std::vector<char>& data, const std::string& s) {
      serialize(data, static_cast<int32_t>(s.size()));
      for (char c : s) {
        data.push_back(c);
      }
    }
    
    static MsgType deserializeMsgType(iter& it) {
      auto m = static_cast<MsgType>(*it);
      ++it;
      return m;
    }
    
    static Field deserializeField(iter& it) {
      auto f = static_cast<Field>(*it);
      ++it;
      return f;
    }
    
    static int32_t deserializeInt(iter& it) {
      auto b1 = static_cast<uint32_t>(reinterpret_cast<uint8_t&>(*it++));
      auto b2 = static_cast<uint32_t>(reinterpret_cast<uint8_t&>(*it++));
      auto b3 = static_cast<uint32_t>(reinterpret_cast<uint8_t&>(*it++));
      auto b4 = static_cast<uint32_t>(reinterpret_cast<uint8_t&>(*it++));
      
      return static_cast<int32_t>(b1 << 24 | b2 << 16 | b3 << 8 | b4);
    }
    
    static NodeStatus deserializeStatus(iter& it) {
      auto f = static_cast<NodeStatus>(*it);
      ++it;
      return f;
    }
    
    static std::string deserializeString(iter& it) {
      std::string result;
      int32_t size = deserializeInt(it);
      result.reserve(static_cast<size_t>(size));
      for (int32_t i = 0; i < size; i++) {
        result += *it;
        ++it;
      }
      return result;
    }
    
  public:
    Message& makeNode(NodeUID node, NodeUID parent,
                      int32_t alt, int32_t kids, NodeStatus status) {
      msg.reset();
      msg.set_type(MsgType::NODE);
      
      msg.set_nodeUID(node);
      msg.set_parentUID(parent);
      
      msg.set_alt(alt);
      msg.set_kids(kids);
      msg.set_status(status);
      
      return msg;
    }
    
    void makeStart(const std::string& info) {
      msg.reset();
      msg.set_type(MsgType::START);
      msg.set_version(PROFILER_PROTOCOL_VERSION);
      msg.set_info(info); /// info containts name, has_restarts, execution id
        }
    
    void makeRestart(const std::string& info) {
      msg.reset();
      msg.set_type(MsgType::RESTART);
      msg.set_info(info); /// info contains restart_id (-1 default)
        }
    
    void makeDone(void) {
      msg.reset();
      msg.set_type(MsgType::DONE);
    }
    
    const Message& get_msg(void) { return msg; }
    
    std::vector<char> serialize(void) const {
      std::vector<char> data;
      size_t dataSize = 1 + (msg.isNode() ? 4 * 8 + 1 : 0) +
        (msg.has_label() ? 1 + 4 + msg.label().size() : 0) +
        (msg.has_nogood() ? 1 + 4 + msg.nogood().size() : 0) +
        (msg.has_info() ? 1 + 4 + msg.info().size() : 0);
      data.reserve(dataSize);
      
      serializeType(data, msg.type());
      if (msg.isNode()) {
        // serialize NodeId node
        auto n_uid = msg.nodeUID();
        serialize(data, n_uid.nid);
        serialize(data, n_uid.rid);
        serialize(data, n_uid.tid);
        // serialize NodeId parent
        auto p_uid = msg.parentUID();
        serialize(data, p_uid.nid);
        serialize(data, p_uid.rid);
        serialize(data, p_uid.tid);
        // Other Data
        serialize(data, msg.alt());
        serialize(data, msg.kids());
        serialize(data, msg.status());
      }
      
      if(msg.has_version()) {
        serializeField(data, VERSION);
        serialize(data, msg.version());
      }
      if (msg.has_label()) {
        serializeField(data, LABEL);
        serialize(data, msg.label());
      }
      if (msg.has_nogood()) {
        serializeField(data, NOGOOD);
        serialize(data, msg.nogood());
      }
      if (msg.has_info()) {
        serializeField(data, INFO);
        serialize(data, msg.info());
      }
      return data;
    }
    
    void deserialize(char* data, size_t size) {
      char *end = data + size;
      msg.set_type(deserializeMsgType(data));
      if (msg.isNode()) {
        int32_t nid = deserializeInt(data);
        int32_t rid = deserializeInt(data);
        int32_t tid = deserializeInt(data);
        
        msg.set_nodeUID({nid, rid, tid});
        
        nid = deserializeInt(data);
        rid = deserializeInt(data);
        tid = deserializeInt(data);
        
        msg.set_parentUID({nid, rid, tid});
        
        msg.set_alt(deserializeInt(data));
        msg.set_kids(deserializeInt(data));
        msg.set_status(deserializeStatus(data));
      }
      
      msg.reset();
      
      while (data != end) {
        MessageMarshalling::Field f = deserializeField(data);
        switch (f) {
        case VERSION:
          msg.set_version(deserializeInt(data)); break;
        case LABEL:
          msg.set_label(deserializeString(data)); break;
        case NOGOOD:
          msg.set_nogood(deserializeString(data)); break;
        case INFO:
          msg.set_info(deserializeString(data)); break;
        default:
          break;
        }
      }
    }
  };

}}

// STATISTICS: search-trace
