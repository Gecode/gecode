/* -*- mode: c++; c-basic-offset: 2; indent-tabs-mode: nil -*- */
#include <gecode/search.hh>
#include <gecode/word.hh>

#include <cstring>
#include <iostream>
#include <vector>

using namespace Gecode;

namespace {
  enum Family { CRC16, XORSHIFT32, SPECK32_64 };
  struct Case {
    const char* id; Family family; unsigned int rounds;
    WordValue known_mask[4], known_value[4];
    WordValue output_mask, output_value; bool exclude;
  };

  const Case cases[] = {
    {"crc-base",CRC16,8,{240,0,0,0},{80,0,0,0},255,35,false},
    {"crc-unknown-bits",CRC16,8,{252,0,0,0},{88,0,0,0},255,35,false},
    {"crc-observation",CRC16,8,{240,0,0,0},{80,0,0,0},65535,14115,false},
    {"crc-rounds",CRC16,4,{240,0,0,0},{80,0,0,0},255,116,false},
    {"crc-excluded-unique",CRC16,8,{255,0,0,0},{90,0,0,0},65535,14115,true},
    {"xorshift-base",XORSHIFT32,1,{4294967280U,0,0,0},{305419888U,0,0,0},255,165,false},
    {"xorshift-unknown-bits",XORSHIFT32,1,{4294967292U,0,0,0},{305419896U,0,0,0},255,165,false},
    {"xorshift-observation",XORSHIFT32,1,{4294967280U,0,0,0},{305419888U,0,0,0},4294967295U,2274908837U,false},
    {"xorshift-rounds",XORSHIFT32,3,{4294967280U,0,0,0},{305419888U,0,0,0},255,196,false},
    {"xorshift-excluded-unique",XORSHIFT32,2,{4294967295U,0,0,0},{305419896U,0,0,0},4294967295U,358294691U,true},
    {"speck-base",SPECK32_64,2,{65520,65535,65535,65535},{256,2312,4368,6424},255,64,false},
    {"speck-unknown-bits",SPECK32_64,2,{65532,65535,65535,65535},{256,2312,4368,6424},255,64,false},
    {"speck-observation",SPECK32_64,2,{65520,65535,65535,65535},{256,2312,4368,6424},4294967295U,937422656U,false},
    {"speck-rounds",SPECK32_64,4,{65520,65535,65535,65535},{256,2312,4368,6424},255,119,false},
    {"speck-excluded-unique",SPECK32_64,3,{65535,65535,65535,65535},{256,2312,4368,6424},4294967295U,3436343761U,true}
  };

  void cube(Home home, WordVar x, unsigned int width, WordValue mask,
            WordValue value) {
    const WordValue all = width == 64 ? ~WordValue(0) : (WordValue(1)<<width)-1;
    dom(home,x,value&mask,(value&mask)|(all&~mask));
  }
  void observe(Home home, WordVar x, unsigned int width, WordValue mask,
               WordValue value) {
    cube(home,x,width,mask,value);
  }

  class Model : public Space {
  public:
    WordVarArray public_words;
    const Case* spec;
    Model(const Case& c, bool msb, const WordValue* excluded=nullptr)
      : public_words(*this,c.family==SPECK32_64 ? 4 : 1,
        c.family==CRC16 ? 8 : (c.family==XORSHIFT32 ? 32 : 16),0,
        c.family==CRC16 ? 255 : (c.family==XORSHIFT32 ? 0xffffffffU : 65535)), spec(&c) {
      for (int i=0; i<public_words.size(); i++)
        cube(*this,public_words[i],public_words[i].width(),c.known_mask[i],c.known_value[i]);
      if (c.family == CRC16) {
        WordVarArray state(*this,c.rounds+1,16,0,65535); dom(*this,state[0],0x1d0fU);
        for (unsigned int i=0; i<c.rounds; i++) {
          BoolVar input(*this,0,1), top(*this,0,1), feedback(*this,0,1);
          channel(*this,public_words[0],7-i,input); channel(*this,state[i],15,top);
          rel(*this,input,BOT_XOR,top,feedback);
          WordVar shifted(*this,16), polynomial(*this,16), zero(*this,16,0,0);
          shift_left(*this,state[i],1,shifted);
          ite(*this,feedback,16,0x1021U,zero,polynomial);
          rel(*this,shifted,WOT_XOR,polynomial,state[i+1]);
        }
        observe(*this,state[c.rounds],16,c.output_mask,c.output_value);
      } else if (c.family == XORSHIFT32) {
        WordVarArray state(*this,3*c.rounds+1,32,0,0xffffffffU); rel(*this,state[0],WRT_EQ,public_words[0]);
        for (unsigned int i=0; i<c.rounds; i++) {
          WordVar shifted0(*this,32), shifted1(*this,32), shifted2(*this,32);
          shift_left(*this,state[3*i],13,shifted0);
          rel(*this,state[3*i],WOT_XOR,shifted0,state[3*i+1]);
          logical_shift_right(*this,state[3*i+1],17,shifted1);
          rel(*this,state[3*i+1],WOT_XOR,shifted1,state[3*i+2]);
          shift_left(*this,state[3*i+2],5,shifted2);
          rel(*this,state[3*i+2],WOT_XOR,shifted2,state[3*i+3]);
        }
        observe(*this,state[3*c.rounds],32,c.output_mask,c.output_value);
      } else {
        WordVarArray keys(*this,c.rounds,16,0,65535), l(*this,c.rounds+2,16,0,65535);
        rel(*this,keys[0],WRT_EQ,public_words[0]);
        rel(*this,l[0],WRT_EQ,public_words[1]); rel(*this,l[1],WRT_EQ,public_words[2]);
        rel(*this,l[2],WRT_EQ,public_words[3]);
        for (unsigned int i=0; i+1<c.rounds; i++) {
          WordVar rotated(*this,16), summed(*this,16), next_l(*this,16), key_rotated(*this,16);
          rotate_right(*this,l[i],7,rotated); add(*this,rotated,keys[i],summed);
          rel(*this,summed,WOT_XOR,16,i,next_l); rel(*this,l[i+3],WRT_EQ,next_l);
          rotate_left(*this,keys[i],2,key_rotated);
          rel(*this,key_rotated,WOT_XOR,next_l,keys[i+1]);
        }
        WordVarArray x(*this,c.rounds+1,16,0,65535), y(*this,c.rounds+1,16,0,65535);
        dom(*this,x[0],0x6574U); dom(*this,y[0],0x694cU);
        for (unsigned int i=0; i<c.rounds; i++) {
          WordVar rotated_x(*this,16), summed(*this,16), rotated_y(*this,16);
          rotate_right(*this,x[i],7,rotated_x); add(*this,rotated_x,y[i],summed);
          rel(*this,summed,WOT_XOR,keys[i],x[i+1]); rotate_left(*this,y[i],2,rotated_y);
          rel(*this,rotated_y,WOT_XOR,x[i+1],y[i+1]);
        }
        observe(*this,x[c.rounds],16,(c.output_mask>>16)&0xffffU,(c.output_value>>16)&0xffffU);
        observe(*this,y[c.rounds],16,c.output_mask&0xffffU,c.output_value&0xffffU);
      }
      if (c.exclude) {
        BoolVarArgs equalities;
        for (int i=0; i<public_words.size(); i++) {
          BoolVar equal(*this,0,1); rel(*this,public_words[i],WRT_EQ,
            public_words[i].width(),excluded == nullptr ? c.known_value[i] : excluded[i],
            Reify(equal,RM_EQV));
          equalities << equal;
        }
        linear(*this,equalities,IRT_LQ,public_words.size()-1);
      }
      if (msb)
        branch(*this,WordVarArgs(public_words),WORD_VAR_NONE(),WORD_VAL_MSB());
      else
        branch(*this,WordVarArgs(public_words),WORD_VAR_NONE(),WORD_VAL_LSB());
    }
    Model(Model& s) : Space(s), spec(s.spec) { public_words.update(*this,s.public_words); }
    Space* copy(void) { return new Model(*this); }
  };
}

int main(int argc, char* argv[]) {
  const bool parameter_mode=(argc >= 21) && !std::strcmp(argv[1],"--parameters");
  const int option_start=parameter_mode ? 19 : 3;
  if ((!parameter_mode && std::strcmp(argv[1],"--case")) || argc < option_start ||
      ((argc-option_start)%2 != 0)) return 2;
  const char* search="lsb";
  unsigned int batch=1U;
  for (int i=option_start; i<argc; i+=2) {
    if (!std::strcmp(argv[i],"--search") &&
        (!std::strcmp(argv[i+1],"lsb") || !std::strcmp(argv[i+1],"msb"))) search=argv[i+1];
    else if (!std::strcmp(argv[i],"--batch"))
      batch=static_cast<unsigned int>(std::strtoul(argv[i+1],nullptr,0));
    else return 2;
  }
  if (batch == 0U) return 2;
  Case dynamic={"dynamic",CRC16,0,{0,0,0,0},{0,0,0,0},0,0,false};
  WordValue dynamic_excluded[4]={0,0,0,0};
  const Case* selected=nullptr;
  if (parameter_mode) {
    if (!std::strcmp(argv[2],"crc16")) dynamic.family=CRC16;
    else if (!std::strcmp(argv[2],"xorshift32")) dynamic.family=XORSHIFT32;
    else if (!std::strcmp(argv[2],"speck32_64")) dynamic.family=SPECK32_64;
    else return 2;
    dynamic.rounds=static_cast<unsigned int>(std::strtoul(argv[3],nullptr,0));
    for (int i=0; i<4; i++) {
      dynamic.known_mask[i]=std::strtoull(argv[4+2*i],nullptr,0);
      dynamic.known_value[i]=std::strtoull(argv[5+2*i],nullptr,0);
    }
    dynamic.output_mask=std::strtoull(argv[12],nullptr,0);
    dynamic.output_value=std::strtoull(argv[13],nullptr,0);
    const unsigned long exclude=std::strtoul(argv[14],nullptr,0);
    dynamic.exclude=exclude != 0U;
    for (int i=0; i<4; i++)
      dynamic_excluded[i]=std::strtoull(argv[15+i],nullptr,0);
    if (dynamic.rounds == 0U || exclude > 1U) return 2;
    const WordValue input_mask=dynamic.family == CRC16 ? 0xffU :
      (dynamic.family == XORSHIFT32 ? 0xffffffffU : 0xffffU);
    const int public_count=dynamic.family == SPECK32_64 ? 4 : 1;
    for (int i=0; i<public_count; i++)
      if ((dynamic.known_mask[i] & ~input_mask) ||
          (dynamic.known_value[i] & ~input_mask) ||
          (dynamic_excluded[i] & ~input_mask)) return 2;
    for (int i=public_count; i<4; i++)
      if (dynamic_excluded[i] != 0U) return 2;
    const WordValue result_mask=dynamic.family == CRC16 ? 0xffffU : 0xffffffffU;
    if ((dynamic.output_mask & ~result_mask) ||
        (dynamic.output_value & ~result_mask)) return 2;
    selected=&dynamic;
  } else {
    for (const Case& c : cases) if (std::strcmp(c.id,argv[2]) == 0) selected=&c;
  }
  if (selected == nullptr) return 2;
  const bool msb=!std::strcmp(search,"msb");
  const WordValue* excluded=parameter_mode ? dynamic_excluded : nullptr;
  Model* root=new Model(*selected,msb,excluded); SpaceStatus root_status=root->status();
  std::vector<unsigned int> root_fixed;
  for (int i=0; i<root->public_words.size(); i++)
    root_fixed.push_back(root->public_words[i].width()-root->public_words[i].unknown_size());
  delete root; std::vector<std::vector<WordValue> > rows;
  std::uint64_t total_solutions=0;
  for (unsigned int trial=0; trial<batch; trial++) {
    Model* trial_root=new Model(*selected,msb,excluded); DFS<Model> engine(trial_root); delete trial_root;
    while (Model* solution=engine.next()) {
      total_solutions++;
      if (trial == 0U) {
        std::vector<WordValue> row;
        for (int i=0; i<solution->public_words.size(); i++) row.push_back(solution->public_words[i].val());
        rows.push_back(row);
      }
      delete solution;
    }
  }
  std::cout << "{\"schema_version\":1,\"case\":\"" << selected->id
            << "\",\"semantic_status\":\"" << (rows.empty()?"unsat":"sat")
            << "\",\"solutions\":" << rows.size() << ",\"batch\":" << batch
            << ",\"batch_solutions\":" << total_solutions << ",\"decision_variables\":[";
  if (selected->family==CRC16) std::cout << "\"message\"";
  else if (selected->family==XORSHIFT32) std::cout << "\"state\"";
  else std::cout << "\"key0\",\"key1\",\"key2\",\"key3\"";
  std::cout << "],\"projections\":[";
  for (std::size_t i=0; i<rows.size(); i++) { if (i) std::cout << ','; std::cout << '[';
    for (std::size_t j=0; j<rows[i].size(); j++) { if (j) std::cout << ','; std::cout << rows[i][j]; }
    std::cout << ']'; }
  std::cout << "],\"root\":{\"status\":\"" << (root_status==SS_FAILED?"failed":(root_status==SS_SOLVED?"solved":"branch"))
            << "\",\"fixed_public_bits\":[";
  for (std::size_t i=0; i<root_fixed.size(); i++) { if (i) std::cout << ','; std::cout << root_fixed[i]; }
  std::cout << "]}}\n";
}

// STATISTICS: example-any
