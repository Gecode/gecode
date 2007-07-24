registry = [
  {
    'name' : "int.distinct.Val",
    'inst' : [
      { 'type' : [']intvar'],
        'post' : ['distinct', 0, 'ICL_VAL']
      },
      { 'type' : [']offsetvar'],
        'post' : ['distinct', (0, 0), (0, 1), 'ICL_VAL']
      }
    ]
  },
  {
    'name' : "int.distinct.Bnd",
    'inst' : [
      { 'type' : [']intvar'],
        'post' : ['distinct', 0, 'ICL_BND']
      },
      { 'type' : [']offsetvar'],
        'post' : ['distinct', (0, 0), (0, 1), 'ICL_BND']
      }
    ]
  },
  {
    'name' : "int.distinct.Dom",
    'inst' : [
      { 'type' : [']intvar'],
        'post' : ['distinct', 0, 'ICL_DOM']
      },
      { 'type' : [']offsetvar'],
        'post' : ['distinct', (0, 0), (0, 1), 'ICL_DOM']
      }
    ]
  },
  {
    'name' : "ViewValAssignment",
    'inst' : []
  },
  {
    'name' : "ViewValBranching",
    'inst' : []
  },
  {
    'name' : "int.AbsBnd",
    'inst' : [
    { 'type' : ['intvar', 'intvar'],
      'post' : ['abs', 0, 1, 'ICL_BND']
    }
    ]
  },
  {
    'name' : "int.AbsDom",
    'inst' : [
    { 'type' : ['intvar', 'intvar'],
      'post' : ['abs', 0, 1, 'ICL_DOM']
    }
    ]
  },
  {
    'name' : "int.Max",
    'inst' : [
    { 'type' : ['intvar', 'intvar', 'intvar'],
      'post' : ['max', 0, 1, 2]
    },
    { 'type' : ['minusvar', 'minusvar', 'minusvar'],
      'post' : ['min', 0, 1, 2]
    }
    ]
  },
  {
    'name' : "int.Mult",
    'inst' : [
    { 'type' : ['intvar', 'intvar', 'intvar'],
      'post' : ['mult', 0, 1, 2]
    },
    ]
  },
  {
    'name' : "int.MultPlus",
    'inst' : [
    { 'type' : ['intvar', 'intvar', 'intvar'],
      'post' : ['mult', 0, 1, 2]
    },
    { 'type' : ['minusvar', 'minusvar', 'intvar'],
      'post' : ['mult', 0, 1, 2]
    },
    { 'type' : ['minusvar', 'intvar', 'minusvar'],
      'post' : ['mult', 0, 1, 2]
    }
    ]
  },
  {
    'name' : "int.NaryMax",
    'inst' : [
    { 'type' : [']intvar', 'intvar'],
      'post' : ['max', 0, 1]
    },
    { 'type' : [']minusvar', 'minusvar'],
      'post' : ['min', 0, 1]
    }
    ]
  },
  {
    'name' : "int.Square",
    'inst' : [
    { 'type' : ['intvar', 'intvar'],
      'post' : ['mult', 0, 0, 1]
    }
    ]
  },
  {
    'name' : "int.SquarePlus",
    'inst' : [
    { 'type' : ['intvar', 'intvar'],
      'post' : ['mult', 0, 0, 1]
    },
    { 'type' : ['minusvar', 'intvar'],
      'post' : ['mult', 0, 0, 1]
    }
    ]
  },
  {
    'name' : "int.bool.BinOrTrue",
    'inst' : [
    { 'type' : ['boolvar', 'boolvar'],
      'post' : ['rel', 0, 'BOT_OR', 1, 'true']
    }
    ]
  },
  {
    'name' : "int.bool.Eq",
    'inst' : [
    { 'type' : ['boolvar', 'boolvar'],
      'post' : ['rel', 0, 'BOT_EQV', 1, 'true']
    },
    { 'type' : ['boolvar', 'negboolvar'],
      'post' : ['rel', 0, 'BOT_XOR', 1, 'true']
    }    
    ]
  },
  {
    'name' : "int.bool.Eqv",
    'inst' : [
    { 'type' : ['boolvar', 'boolvar', 'boolvar'],
      'post' : ['rel', 0, 'BOT_EQV', 1, 2]
    },
    { 'type' : ['boolvar', 'boolvar', 'negboolvar'],
      'post' : ['rel', 0, 'BOT_XOR', 1, 2]
    }    
    ]
  },
  {
    'name' : "int.bool.Lq",
    'inst' : [
    { 'type' : ['boolvar', 'boolvar'],
      'post' : ['rel', 0, 'IRT_LQ', 1]
    }
    ]
  },
  {
    'name' : "int.bool.NaryOr",
    'inst' : [
    { 'type' : [']boolvar', 'boolvar'],
      'post' : ['rel', 0, 'BOT_OR', 1]
    },
    { 'type' : [']negboolvar', 'negboolvar'],
      'post' : ['rel', 0, 'BOT_AND', 1]
    }
    ]
  },
  {
    'name' : "int.bool.NaryOrTrue",
    'inst' : [
    { 'type' : [']boolvar', 'boolvar'],
      'post' : ['rel', 0, 'BOT_OR', 'true']
    },
    { 'type' : [']negboolvar', 'negboolvar'],
      'post' : ['rel', 0, 'BOT_AND', 'true']
    }
    ]
  },
  {
    'name' : "int.bool.Or",
    'inst' : [
    { 'type' : ['boolvar', 'boolvar', 'boolvar'],
      'post' : ['rel', 0, 'BOT_OR', 1, 2]
    },
    { 'type' : ['negboolvar', 'negboolvar', 'negboolvar'],
      'post' : ['rel', 0, 'BOT_AND', 1, 2]
    },
    { 'type' : ['negboolvar', 'boolvar', 'boolvar'],
      'post' : ['rel', 0, 'BOT_IMP', 1, 2]
    }
    ]
  },
  {
    'name' : "int.bool.OrTrueSubsumed",
    'inst' : []
  },
  {
    'name' : "int.bool.QuadOrTrue",
    'inst' : [
    { 'type' : ['boolvar', 'boolvar', 'boolvar', 'boolvar'],
      'post' : ['rel', [0,1,2,3], 'BOT_OR', 'true']
    },
    { 'type' : ['negboolvar', 'negboolvar', 'negboolvar', 'negboolvar'],
      'post' : ['rel', [0,1,2,3], 'BOT_AND', 'true']
    }
    ]
  },
  {
    'name' : "int.bool.TerOrTrue",
    'inst' : [
    { 'type' : ['boolvar', 'boolvar', 'boolvar'],
      'post' : ['rel', [0,1,2], 'BOT_OR', 'true']
    },
    { 'type' : ['negboolvar', 'negboolvar', 'negboolvar'],
      'post' : ['rel', [0,1,2], 'BOT_AND', 'true']
    }
    ]
  },
  {
    'name' : "int.channel.Dom",
    'inst' : [
    { 'type' : [']intvar', ']intvar'],
      'post' : ['channel', 0, 1, 'ICL_DOM']
    }    
    ]
  },
  {
    'name' : "int.channel.LinkMulti",
    'inst' : [
    { 'type' : [']boolvar', 'intvar', 'int'],
      'post' : ['channel', 0, 1, 2]
    }    
    ]
  },
  {
    'name' : "int.channel.LinkSingle",
    'inst' : [
    { 'type' : ['boolvar', 'intvar'],
      'post' : ['channel', 0, 1]
    }    
    ]
  },
  {
    'name' : "int.channel.Val",
    'inst' : [
    { 'type' : [']intvar', ']intvar'],
      'post' : ['channel', 0, 1, 'ICL_VAL']
    }    
    ]
  },
  {
    'name' : "int.circuit.Dom",
    'inst' : [
    { 'type' : [']intvar'],
      'post' : ['circuit', 'ICL_DOM']
    }    
    ]
  },
  {
    'name' : "int.circuit.Val",
    'inst' : [
    { 'type' : [']intvar'],
      'post' : ['circuit', 'ICL_DOM']
    }    
    ]
  },
  {
    'name' : "int.count.EqInt",
    'inst' : [
    { 'type' : [']intvar', 'intvar', 'int'],
      'post' : ['count', 0, 1, 'IRT_EQ', 2]
    },
    { 'type' : [']intvar', 'constint', 'int'],
      'post' : ['count', 0, 1, 'IRT_EQ', 2]
    },
    { 'type' : [']offsetvar', 'constint', 'int'],
      'post' : ['count', (0, 1), (0, 0), 'IRT_EQ', 2]
    }
    ]
  },
  {
    'name' : "int.count.EqView",
    'inst' : [
    { 'type' : [']intvar', 'intvar', 'intvar', 'int'],
      'post' : ['my_count', 0, 1, 'IRT_EQ', 2, 3]
    },
    { 'type' : [']intvar', 'constint', 'intvar', 'int'],
      'post' : ['my_count', 0, 1, 'IRT_EQ', 2, 3]
    },
    { 'type' : [']offsetvar', 'constint', 'intvar', 'int'],
      'post' : ['my_count', (0, 1), (0, 0), 'IRT_EQ', 2, 3]
    }
    ]
  },
  {
    'name' : "int.count.GqInt",
    'inst' : [
    { 'type' : [']intvar', 'intvar', 'int'],
      'post' : ['count', 0, 1, 'IRT_GQ', 2]
    },
    { 'type' : [']intvar', 'constint', 'int'],
      'post' : ['count', 0, 1, 'IRT_GQ', 2]
    },
    { 'type' : [']offsetvar', 'constint', 'int'],
      'post' : ['count', (0, 1), (0, 0), 'IRT_GQ', 2]
    }
    ]
  },
  {
    'name' : "int.count.GqView",
    'inst' : [
    { 'type' : [']intvar', 'intvar', 'intvar', 'int'],
      'post' : ['my_count', 0, 1, 'IRT_GQ', 2, 3]
    },
    { 'type' : [']intvar', 'constint', 'intvar', 'int'],
      'post' : ['my_count', 0, 1, 'IRT_GQ', 2, 3]
    },
    { 'type' : [']offsetvar', 'constint', 'intvar', 'int'],
      'post' : ['my_count', (0, 1), (0, 0), 'IRT_GQ', 2, 3]
    }
    ]
  },
  {
    'name' : "int.count.LqInt",
    'inst' : [
    { 'type' : [']intvar', 'intvar', 'int'],
      'post' : ['count', 0, 1, 'IRT_LQ', 2]
    },
    { 'type' : [']intvar', 'constint', 'int'],
      'post' : ['count', 0, 1, 'IRT_LQ', 2]
    },
    { 'type' : [']offsetvar', 'constint', 'int'],
      'post' : ['count', (0, 1), (0, 0), 'IRT_LQ', 2]
    }
    ]
  },
  {
    'name' : "int.count.LqView",
    'inst' : [
    { 'type' : [']intvar', 'intvar', 'intvar', 'int'],
      'post' : ['my_count', 0, 1, 'IRT_LQ', 2, 3]
    },
    { 'type' : [']intvar', 'constint', 'intvar', 'int'],
      'post' : ['my_count', 0, 1, 'IRT_LQ', 2, 3]
    },
    { 'type' : [']offsetvar', 'constint', 'intvar', 'int'],
      'post' : ['my_count', (0, 1), (0, 0), 'IRT_LQ', 2, 3]
    }
    ]
  },
  {
    'name' : "int.count.NqInt",
    'inst' : [
    { 'type' : [']intvar', 'intvar', 'int'],
      'post' : ['count', 0, 1, 'IRT_NQ', 2]
    },
    { 'type' : [']intvar', 'constint', 'int'],
      'post' : ['count', 0, 1, 'IRT_NQ', 2]
    },
    { 'type' : [']offsetvar', 'constint', 'int'],
      'post' : ['count', (0, 1), (0, 0), 'IRT_NQ', 2]
    }
    ]
  },
  {
    'name' : "int.count.NqView",
    'inst' : [
    { 'type' : [']intvar', 'intvar', 'intvar', 'int'],
      'post' : ['my_count', 0, 1, 'IRT_NQ', 2, 3]
    },
    { 'type' : [']intvar', 'constint', 'intvar', 'int'],
      'post' : ['my_count', 0, 1, 'IRT_NQ', 2, 3]
    },
    { 'type' : [']offsetvar', 'constint', 'intvar', 'int'],
      'post' : ['my_count', (0, 1), (0, 0), 'IRT_NQ', 2, 3]
    }
    ]
  },
  {
    'name' : "int.cumulatives.Val",
    'inst' : []
  },
  {
    'name' : "int.distinct.TerDom",
    'inst' : [
    { 'type' : ['intvar','intvar','intvar'],
      'post' : ['distinct', [0,1,2], 'ICL_DOM']
    },
    { 'type' : ['offsetvar','offsetvar','offsetvar'],
      'post' : ['my_distinct', (0,0), (1,0), (2,0), (0,1), (1,1), (2,1), 'ICL_DOM']
    }
    ]
  },
  {
    'name' : "int.dom.ReIntSet",
    'inst' : [
    { 'type' : ['intvar', 'boolvar', 'intset'],
      'post' : ['dom', 0, 2, 1]
    }
    ]
  },
  {
    'name' : "int.dom.ReRange",
    'inst' : [
    { 'type' : ['intvar', 'boolvar', 'int', 'int'],
      'post' : ['dom', 0, 2, 3, 1]
    }
    ]
  },
  {
    'name' : "int.element.Int",
    'inst' : [
    { 'type' : ['intvar', 'intvar', ']int'],
      'post' : ['element', 2, 0, 1]
    },
    { 'type' : ['intvar', 'boolvar', ']int'],
      'post' : ['element', 2, 0, 1]
    },
    { 'type' : ['intvar', 'constint', ']int'],
      'post' : ['element', 2, 0, 1]
    },
    { 'type' : ['offsetvar', 'intvar', ']int'],
      'post' : ['element', 2, (0,1), 1, (0,0)]
    },
    { 'type' : ['offsetvar', 'boolvar', ']int'],
      'post' : ['element', 2, (0,1), 1, (0,0)]
    },
    { 'type' : ['offsetvar', 'constint', ']int'],
      'post' : ['element', 2, (0,1), 1, (0,0)]
    }
    ]
  },
  {
    'name' : "int.element.ViewBnd",
    'inst' : [
    { 'type' : ['intvar', 'intvar', ']int', ']intvar'],
      'post' : ['my_element', 2, 3, 0, 1, '0', 'ICL_BND']
    },
    { 'type' : ['intvar', 'constint', ']int', ']intvar'],
      'post' : ['my_element', 2, 3, 0, 1, '0', 'ICL_BND']
    },
    { 'type' : ['intvar', 'boolvar', ']int', ']boolvar'],
      'post' : ['my_element', 2, 3, 0, 1, '0', 'ICL_BND']
    },
    { 'type' : ['intvar', 'constint', ']int', ']boolvar'],
      'post' : ['my_element', 2, 3, 0, 1, '0', 'ICL_BND']
    },
    { 'type' : ['offsetvar', 'intvar', ']int', ']intvar'],
      'post' : ['my_element', 2, 3, (0,1), 1, (0,0), 'ICL_BND']
    },
    { 'type' : ['offsetvar', 'constint', ']int', ']intvar'],
      'post' : ['my_element', 2, 3, (0,1), 1, (0,0), 'ICL_BND']
    },
    { 'type' : ['offsetvar', 'boolvar', ']int', ']boolvar'],
      'post' : ['my_element', 2, 3, (0,1), 1, (0,0), 'ICL_BND']
    },
    { 'type' : ['offsetvar', 'constint', ']int', ']boolvar'],
      'post' : ['my_element', 2, 3, (0,1), 1, (0,0), 'ICL_BND']
    }    
    ]
  },
  {
    'name' : "int.element.ViewDom",
    'inst' : [
    { 'type' : ['intvar', 'intvar', ']int', ']intvar'],
      'post' : ['my_element', 2, 3, 0, 1, '0', 'ICL_DOM']
    },
    { 'type' : ['intvar', 'constint', ']int', ']intvar'],
      'post' : ['my_element', 2, 3, 0, 1, '0', 'ICL_DOM']
    },
    { 'type' : ['intvar', 'boolvar', ']int', ']boolvar'],
      'post' : ['my_element', 2, 3, 0, 1, '0', 'ICL_DOM']
    },
    { 'type' : ['intvar', 'constint', ']int', ']boolvar'],
      'post' : ['my_element', 2, 3, 0, 1, '0', 'ICL_DOM']
    },
    { 'type' : ['offsetvar', 'intvar', ']int', ']intvar'],
      'post' : ['my_element', 2, 3, (0,1), 1, (0,0), 'ICL_DOM']
    },
    { 'type' : ['offsetvar', 'constint', ']int', ']intvar'],
      'post' : ['my_element', 2, 3, (0,1), 1, (0,0), 'ICL_DOM']
    },
    { 'type' : ['offsetvar', 'boolvar', ']int', ']boolvar'],
      'post' : ['my_element', 2, 3, (0,1), 1, (0,0), 'ICL_DOM']
    },
    { 'type' : ['offsetvar', 'constint', ']int', ']boolvar'],
      'post' : ['my_element', 2, 3, (0,1), 1, (0,0), 'ICL_DOM']
    }
    ]
  },
  {
    'name' : "int.extensional.Basic",
    'inst' : []
  },
  {
    'name' : "int.extensional.Incremental",
    'inst' : []
  },
  {
    'name' : "int.extensional.Val",
    'inst' : []
  },
  {
    'name' : "int.gcc.Bnd",
    'inst' : []
  },
  {
    'name' : "int.gcc.Dom",
    'inst' : []
  },
  {
    'name' : "int.gcc.Val",
    'inst' : []
  },
  {
    'name' : "int.linear.DomEq",
    'inst' : []
  },
  {
    'name' : "int.linear.Eq",
    'inst' : []
  },
  {
    'name' : "int.linear.EqBin",
    'inst' : []
  },
  {
    'name' : "int.linear.EqBoolInt.memory",
    'inst' : []
  },
  {
    'name' : "int.linear.EqBoolInt.speed",
    'inst' : []
  },
  {
    'name' : "int.linear.EqBoolScale",
    'inst' : []
  },
  {
    'name' : "int.linear.EqBoolView",
    'inst' : []
  },
  {
    'name' : "int.linear.EqTer",
    'inst' : []
  },
  {
    'name' : "int.linear.GqBin",
    'inst' : []
  },
  {
    'name' : "int.linear.GqBoolInt.Speed",
    'inst' : []
  },
  {
    'name' : "int.linear.GqBoolInt.memory",
    'inst' : []
  },
  {
    'name' : "int.linear.GqBoolView",
    'inst' : []
  },
  {
    'name' : "int.linear.Lq",
    'inst' : []
  },
  {
    'name' : "int.linear.LqBin",
    'inst' : []
  },
  {
    'name' : "int.linear.LqBoolScale",
    'inst' : []
  },
  {
    'name' : "int.linear.LqTer",
    'inst' : []
  },
  {
    'name' : "int.linear.Nq",
    'inst' : []
  },
  {
    'name' : "int.linear.NqBin",
    'inst' : []
  },
  {
    'name' : "int.linear.NqBoolInt",
    'inst' : []
  },
  {
    'name' : "int.linear.NqBoolScale",
    'inst' : []
  },
  {
    'name' : "int.linear.NqBoolView",
    'inst' : []
  },
  {
    'name' : "int.linear.NqTer",
    'inst' : []
  },
  {
    'name' : "int.linear.ReEq",
    'inst' : []
  },
  {
    'name' : "int.linear.ReEqBin",
    'inst' : []
  },
  {
    'name' : "int.linear.ReLq",
    'inst' : []
  },
  {
    'name' : "int.linear.ReLqBin",
    'inst' : []
  },
  {
    'name' : "int.regular.Dom",
    'inst' : []
  },
  {
    'name' : "int.rel.EqBnd",
    'inst' : [
    { 'type' : ['intvar', 'intvar'],
      'post' : ['rel', 0, 'IRT_EQ', 1, 'ICL_BND']
    }
    ]
  },
  {
    'name' : "int.rel.EqDom",
    'inst' : [
    { 'type' : ['intvar', 'intvar'],
      'post' : ['rel', 0, 'IRT_EQ', 1, 'ICL_DOM']
    }
    ]
  },
  {
    'name' : "int.rel.Le",
    'inst' : [
    { 'type' : ['intvar', 'intvar'],
      'post' : ['rel', 0, 'IRT_LE', 1]
    }
    ]
  },
  {
    'name' : "int.rel.Lex",
    'inst' : []
  },
  {
    'name' : "int.rel.LexStrict",
    'inst' : []
  },
  {
    'name' : "int.rel.Lq",
    'inst' : [
      { 'type' : ['intvar','intvar'],
        'post' : ['rel', 0, 'IRT_LQ', 1]
      },
      { 'type' : ['boolvar','boolvar'],
        'post' : ['rel', 0, 'IRT_LQ', 1]
      }
    ]
  },
  {
    'name' : "int.rel.NaryEqBnd",
    'inst' : [
    { 'type' : [']intvar'],
      'post' : ['rel', 0, 'IRT_EQ', 'ICL_BND']
    }
    ]
  },
  {
    'name' : "int.rel.NaryEqDom",
    'inst' : [
    { 'type' : [']intvar'],
      'post' : ['rel', 0, 'IRT_EQ', 'ICL_DOM']
    }
    ]
  },
  {
    'name' : "int.rel.NaryNq",
    'inst' : [
    { 'type' : [']intvar'],
      'post' : ['rel', 0, 'IRT_NQ', 'ICL_DOM']
    },
    { 'type' : [']boolvar'],
      'post' : ['rel', 0, 'IRT_NQ', 'ICL_DOM']
    }
    ]
  },
  {
    'name' : "int.rel.Nq",
    'inst' : [
    { 'type' : ['intvar','intvar'],
      'post' : ['rel', 0, 'IRT_NQ', 1]
    },
    { 'type' : ['offsetvar','offsetvar'],
      'post' : ['my_distinct', (0,0), (1,0), (0,1), (1,1), 'ICL_DEF']
    }
    ]
  },
  {
    'name' : "int.rel.ReEqBnd",
    'inst' : []
  },
  {
    'name' : "int.rel.ReEqBndInt",
    'inst' : []
  },
  {
    'name' : "int.rel.ReEqDom",
    'inst' : []
  },
  {
    'name' : "int.rel.ReEqDomInt",
    'inst' : []
  },
  {
    'name' : "int.rel.ReLq",
    'inst' : []
  },
  {
    'name' : "int.rel.ReLqInt",
    'inst' : []
  },
  {
    'name' : "int.sortedness.Sortedness",
    'inst' : []
  },
  {
    'name' : "set.convex.Convex",
    'inst' : [
    { 'type' : ['setvar'],
      'post' : ['convex', 0]
    }
    ]
  },
  {
    'name' : "set.convex.ConvexHull",
    'inst' : [
    { 'type' : ['setvar', 'setvar'],
      'post' : ['convexHull', 0, 1]
    }
    ]
  },
  {
    'name' : "set.distinct.AtmostOne",
    'inst' : [
    { 'type' : [']setvar', 'int'],
      'post' : ['atmostOne', 0, 1]
    }
    ]
  },
  {
    'name' : "set.distinct.Distinct",
    'inst' : [
    { 'type' : [']setvar', 'int'],
      'post' : ['distinct', 0, 1]
    }
    ]
  },
  {
    'name' : "set.int.Card",
    'inst' : [
    { 'type' : ['setvar', 'intvar'],
      'post' : ['cardinality', 0, 1]
    }
    ]
  },
  {
    'name' : "set.int.Channel",
    'inst' : [
    { 'type' : [']intvar', ']setvar'],
      'post' : ['channel', 0, 1]
    }
    ]
  },
  {
    'name' : "set.int.Match",
    'inst' : [
    { 'type' : ['setvar', ']intvar'],
      'post' : ['match', 0, 1]
    }
    ]
  },
  {
    'name' : "set.int.MaxElement",
    'inst' : [
    { 'type' : ['setvar', 'intvar'],
      'post' : ['max', 0, 1]
    }
    ]
  },
  {
    'name' : "set.int.MinElement",
    'inst' : [
    { 'type' : ['setvar', 'intvar'],
      'post' : ['min', 0, 1]
    }
    ]
  },
  {
    'name' : "set.int.Weights",
    'inst' : [
    { 'type' : [']int', ']int', 'setvar', 'intvar'],
      'post' : ['weights', 0, 1, 2, 3]
    }
    ]
  },
  {
    'name' : "set.projectors.CardProjection",
    'inst' : []
  },
  {
    'name' : "set.projectors.ReNaryProjection",
    'inst' : []
  },
  {
    'name' : "set.rel.Distinct",
    'inst' : [
    { 'type' : ['setvar', 'setvar'],
      'post' : ['rel', 0, 'SRT_NQ', 1]
    }
    ]
  },
  {
    'name' : "set.rel.DistinctDoit",
    'inst' : [
    { 'type' : ['setvar', 'setvar'],
      'post' : ['rel', 0, 'SRT_NQ', 1]
    }
    ]
  },
  {
    'name' : "set.rel.Eq",
    'inst' : [
    { 'type' : ['setvar', 'setvar'],
      'post' : ['rel', 0, 'SRT_EQ', 1]
    }
    ]
  },
  {
    'name' : "set.rel.NoSubSet",
    'inst' : []
  },
  {
    'name' : "set.rel.ReEq",
    'inst' : [
    { 'type' : ['setvar', 'setvar', 'boolvar'],
      'post' : ['rel', 0, 'SRT_EQ', 1, 2]
    }
    ]
  },
  {
    'name' : "set.rel.ReSubset",
    'inst' : [
    { 'type' : ['setvar', 'setvar', 'boolvar'],
      'post' : ['rel', 0, 'SRT_SUB', 1, 2]
    }
    ]
  },
  {
    'name' : "set.rel.SubSet",
    'inst' : [
    { 'type' : ['setvar', 'setvar'],
      'post' : ['rel', 0, 'SRT_SUB', 1]
    }
    ]
  },
  {
    'name' : "set.relop.Intersection",
    'inst' : [
    { 'type' : ['setvar', 'setvar', 'setvar'],
      'post' : ['rel', 0, 'SOT_INTER', 1, 'SRT_EQ', 2]
    }
    ]
  },
  {
    'name' : "set.relop.IntersectionN",
    'inst' : [
    { 'type' : [']setvar', 'setvar'],
      'post' : ['rel', 'SOT_INTER', 0, 1]
    }
    ]
  },
  {
    'name' : "set.relop.PartitionN",
    'inst' : [
    { 'type' : [']setvar', 'setvar'],
      'post' : ['rel', 'SOT_DUNION', 0, 1]
    }
    ]
  },
  {
    'name' : "set.relop.SubOfUnion",
    'inst' : [
    { 'type' : ['setvar', 'setvar', 'setvar'],
      'post' : ['rel', 0, 'SOT_UNION', 1, 'SRT_SUP', 2]
    }
    ]
  },
  {
    'name' : "set.relop.SuperOfInter",
    'inst' : [
    { 'type' : ['setvar', 'setvar', 'setvar'],
      'post' : ['rel', 0, 'SOT_INTER', 1, 'SRT_SUB', 2]
    }
    ]
  },
  {
    'name' : "set.relop.Union",
    'inst' : [
    { 'type' : ['setvar', 'setvar', 'setvar'],
      'post' : ['rel', 0, 'SOT_UNION', 1, 'SRT_EQ', 2]
    }
    ]
  },
  {
    'name' : "set.relop.UnionN",
    'inst' : [
    { 'type' : [']setvar', 'setvar', 'intset'],
      'post' : ['rel', 'SOT_UNION', 0, 2, 1]
    }
    ]
  },
  {
    'name' : "set.select.Disjoint",
    'inst' : [
    ]
  },
  {
    'name' : "set.select.Intersection",
    'inst' : []
  },
  {
    'name' : "set.select.Union",
    'inst' : []
  },
  {
    'name' : "set.select.UnionConst",
    'inst' : []
  },
  {
    'name' : "set.sequence.Sequence",
    'inst' : [
    { 'type' : [']setvar'],
      'post' : ['sequence', 0]
    }
    ]
  },
  {
    'name' : "set.sequence.Union",
    'inst' : [
    { 'type' : [']setvar', 'setvar'],
      'post' : ['sequentialUnion', 0, 1]
    }
    ]
  }
  ]
