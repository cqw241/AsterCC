// Standalone C++20 Stage 0. Mechanically synchronized; see scripts/sync_seed.py.
#include "support.hpp"
struct StringMap {
    Arr<str> keys;
    Arr<i64> values;
    Arr<i64> used;
    Arr<i64> count;
};
struct Options {
    str input;
    str output;
    str emit;
    i64 opt;
    i64 llvm;
    str passes;
    bool verify;
    str stats;
    str trace;
    bool check;
};
struct Token {
    str text;
    i64 kind;
    i64 pos;
    i64 file;
};
struct Node {
    str tag;
    str text;
    str ty;
    i64 a;
    i64 b;
    i64 c;
    Arr<i64> kids;
    i64 tok;
};
struct Function {
    str name;
    str module;
    Arr<str> names;
    Arr<str> types;
    str ret;
    i64 body;
    i64 tok;
};
struct Record {
    str name;
    Arr<str> names;
    Arr<str> types;
    i64 tok;
};
struct Import {
    i64 owner;
    str alias;
    str module;
};
struct State {
    Arr<Token> tokens;
    Arr<Node> nodes;
    Arr<Function> functions;
    Arr<Record> records;
    Arr<Import> imports;
    Arr<str> paths;
    Arr<str> sources;
    Arr<str> modules;
    Arr<i64> cursor;
    str root;
    StringMap symbols;
};
struct Inst {
    str op;
    str ty;
    i64 a;
    i64 b;
    Arr<i64> args;
    Arr<i64> labels;
    str text;
    i64 block;
    i64 repl;
    bool dead;
};
struct Block {
    Arr<i64> code;
    Arr<i64> preds;
    Arr<i64> defs;
    Arr<i64> pending;
    bool sealed;
    str term;
    Arr<i64> values;
    Arr<i64> targets;
    bool alive;
};
struct Variable {
    str name;
    str ty;
    bool writable;
    i64 depth;
};
struct IR {
    State s;
    i64 fid;
    Arr<Inst> ins;
    Arr<Block> blocks;
    Arr<Variable> vars;
    Arr<i64> active;
    Arr<i64> current;
    Arr<i64> depth;
    Arr<i64> breaks;
    Arr<i64> continues;
};
struct Value {
    i64 id;
    str ty;
};
struct Lens {
    str kind;
    str ty;
    i64 base;
    i64 index;
    i64 field;
};
StringMap map_new();
i64 map_hash(str key);
i64 map_get(StringMap m, str key);
void map_put(StringMap m, str key, i64 value);
bool alpha(i64 c);
bool digit(i64 c);
i64 lex(State s, i64 file);
i64 precedence(str op);
i64 expression(State s, i64 minp);
i64 parse_block(State s);
i64 statement(State s);
str module_file(str root, str module);
void load_module(State s, str path, str module);
str llvm_type(State s, str t);
str llvm_value(IR g, i64 id);
str typed(IR g, i64 id);
str llvm_symbol(State s, i64 fid);
str hex_digit(i64 n);
str escaped_bytes(str text);
str llvm_literals(IR g);
str llvm_inst(IR g, i64 id);
str llvm_function(IR g);
str llvm_prelude(State s);
str json_quote(str s);
Options parse_options(Arr<str> argv);
Arr<str> split_commas(str s);
str pass_requires(str p);
str pass_invalidates(str p);
void optimize(IR g, Options o, Arr<str> stats);
str dump_ast(State s);
i64 aster_main();
i64 byte(str s, i64 i);
bool starts(str s, str prefix);
bool ends(str s, str suffix);
bool has(Arr<i64> xs, i64 x);
i64 find(Arr<str> xs, str x);
Arr<i64> copy_ids(Arr<i64> xs);
State new_state(str root);
void error(State s, i64 tok, str message);
i64 node(State s, str tag, str text, str ty, i64 a, i64 b, i64 c, Arr<i64> kids, i64 tok);
str peek(State s);
str take(State s);
bool eat(State s, str t);
void need(State s, str t);
str name(State s);
str qualified(State s);
str parse_type(State s);
i64 record_id(State s, str n);
str element(str t);
bool is_integer(str t);
bool known_type(State s, str t);
bool pure(str op);
Arr<i64> operands(Inst x);
bool live_inst(IR g, i64 id);
i64 count_ir(IR g);
str instruction_key(IR g, i64 id);
str dump_ir(IR g);
bool foldable(IR g, Inst x);
i64 eval_scalar(str op, i64 a, i64 b, str ty, str source);
void turn_constant(IR g, i64 id, i64 n);
void simplify(IR g);
void rebuild_cfg(IR g);
void cfg_simplify(IR g);
void dce(IR g);
void sccp(IR g);
Arr<Arr<i64>> dominators(IR g);
i64 position(IR g, i64 id);
Arr<i64> dominator_order(IR g, Arr<Arr<i64>> dom);
void gvn(IR g);
void licm(IR g);
void verify_ir(IR g);
IR new_ir(State s, i64 fid);
i64 new_block(IR g);
i64 emit(IR g, str op, str ty, i64 a, i64 b, Arr<i64> xs, str text);
i64 canonical(IR g, i64 id);
i64 constant(IR g, str ty, i64 n);
void write_var(IR g, i64 block, i64 varid, i64 value);
i64 trivial_phi(IR g, i64 id);
i64 make_phi(IR g, i64 block, i64 varid);
i64 read_var(IR g, i64 block, i64 varid);
void seal(IR g, i64 block);
void terminate(IR g, str term, Arr<i64> values, Arr<i64> targets);
i64 declare(IR g, str n, str ty, bool mut, i64 tok);
i64 lookup(IR g, str n);
void expect(IR g, str actual, str wanted, i64 tok);
i64 function_id(State s, str text, i64 owner);
bool enum_type(State s, str t);
i64 field_id(IR g, str ty, str field, i64 tok);
str builtin_ret(str n);
Arr<str> builtin_types(str n);
Value lower_call(IR g, Node n);
Value lower_short(IR g, Node n);
Value lower_expr_impl(IR g, i64 id, str hint);
Value lower_expr(IR g, i64 id, str hint);
Value lvalue(IR g, i64 id, Arr<Lens> path);
void lower_statement(IR g, i64 id);
void check_layout(State s, str ty, Arr<str> stack, i64 tok);
IR lower_function(State s, i64 fid);
StringMap map_new() {
    StringMap m = StringMap{{}, {}, {}, {i64(0)}};
    i64 i = i64(0);
    while ((i < i64(16))) {
        push(m.keys, str(""));
        push(m.values, wrap_sub(0, i64(1)));
        push(m.used, i64(0));
        i = wrap_add(i, i64(1));
    }
    return m;
}
i64 map_hash(str key) {
    i64 h = i64(1469598103934665603);
    i64 i = i64(0);
    while ((i < len(key))) {
        h = wrap_mul(((h ^ (static_cast<i64>(index(key, i))))), i64(1099511628211));
        i = wrap_add(i, i64(1));
    }
    return h;
}
i64 map_get(StringMap m, str key) {
    i64 slot = (map_hash(key) & (wrap_sub(len(m.keys), i64(1))));
    while ((index(m.used, slot) != i64(0))) {
        if ((index(m.keys, slot) == key)) {
            return index(m.values, slot);
        }
        slot = ((wrap_add(slot, i64(1))) & (wrap_sub(len(m.keys), i64(1))));
    }
    return wrap_sub(0, i64(1));
}
void map_put(StringMap m, str key, i64 value) {
    if ((wrap_mul((wrap_add(index(m.count, i64(0)), i64(1))), i64(3)) >
         wrap_mul(len(m.keys), i64(2)))) {
        i64 i = i64(0);
        Arr<str> keys = {};
        Arr<i64> values = {};
        while ((i < len(m.keys))) {
            if ((index(m.used, i) != i64(0))) {
                push(keys, index(m.keys, i));
                push(values, index(m.values, i));
            }
            i = wrap_add(i, i64(1));
        }
        i64 cap = wrap_mul(len(m.keys), i64(2));
        i = i64(0);
        while ((i < len(m.keys))) {
            index(m.keys, i) = str("");
            index(m.values, i) = wrap_sub(0, i64(1));
            index(m.used, i) = i64(0);
            i = wrap_add(i, i64(1));
        }
        while ((i < cap)) {
            push(m.keys, str(""));
            push(m.values, wrap_sub(0, i64(1)));
            push(m.used, i64(0));
            i = wrap_add(i, i64(1));
        }
        index(m.count, i64(0)) = i64(0);
        i = i64(0);
        while ((i < len(keys))) {
            map_put(m, index(keys, i), index(values, i));
            i = wrap_add(i, i64(1));
        }
    }
    i64 slot = (map_hash(key) & (wrap_sub(len(m.keys), i64(1))));
    while (((index(m.used, slot) != i64(0)) && (index(m.keys, slot) != key))) {
        slot = ((wrap_add(slot, i64(1))) & (wrap_sub(len(m.keys), i64(1))));
    }
    if ((index(m.used, slot) == i64(0))) {
        index(m.count, i64(0)) = wrap_add(index(m.count, i64(0)), i64(1));
    }
    index(m.used, slot) = i64(1);
    index(m.keys, slot) = key;
    index(m.values, slot) = value;
}
bool alpha(i64 c) {
    return ((((c >= i64(65)) && (c <= i64(90))) || ((c >= i64(97)) && (c <= i64(122)))) ||
            (c == i64(95)));
}
bool digit(i64 c) {
    return ((c >= i64(48)) && (c <= i64(57)));
}
i64 lex(State s, i64 file) {
    str src = index(s.sources, file);
    i64 first = len(s.tokens);
    i64 i = i64(0);
    while ((i < len(src))) {
        i64 c = byte(src, i);
        i64 begin = i;
        i64 kind = i64(4);
        str text = str("");
        if (((((c == i64(32)) || (c == i64(9))) || (c == i64(10))) || (c == i64(13)))) {
            i = wrap_add(i, i64(1));
            continue;
        }
        if ((((c == i64(47)) && (wrap_add(i, i64(1)) < len(src))) &&
             (byte(src, wrap_add(i, i64(1))) == i64(47)))) {
            while (((i < len(src)) && (byte(src, i) != i64(10)))) {
                i = wrap_add(i, i64(1));
            }
            continue;
        }
        if ((((c == i64(47)) && (wrap_add(i, i64(1)) < len(src))) &&
             (byte(src, wrap_add(i, i64(1))) == i64(42)))) {
            i = wrap_add(i, i64(2));
            i64 level = i64(1);
            while (((i < len(src)) && (level > i64(0)))) {
                if (((wrap_add(i, i64(1)) < len(src)) && (slice(src, i, i64(2)) == str("/*")))) {
                    level = wrap_add(level, i64(1));
                    i = wrap_add(i, i64(2));
                } else {
                    if (((wrap_add(i, i64(1)) < len(src)) &&
                         (slice(src, i, i64(2)) == str("*/")))) {
                        level = wrap_sub(level, i64(1));
                        i = wrap_add(i, i64(2));
                    } else {
                        i = wrap_add(i, i64(1));
                    }
                }
            }
            if ((level != i64(0))) {
                push(s.tokens, Token{str(""), i64(0), begin, file});
                error(s, wrap_sub(len(s.tokens), i64(1)), str("unterminated comment"));
            }
            continue;
        }
        if (alpha(c)) {
            kind = i64(1);
            i = wrap_add(i, i64(1));
            while (((i < len(src)) && ((alpha(byte(src, i)) || digit(byte(src, i)))))) {
                i = wrap_add(i, i64(1));
            }
            text = slice(src, begin, wrap_sub(i, begin));
        } else {
            if (digit(c)) {
                kind = i64(2);
                i = wrap_add(i, i64(1));
                while (((i < len(src)) && ((alpha(byte(src, i)) || digit(byte(src, i)))))) {
                    i = wrap_add(i, i64(1));
                }
                text = slice(src, begin, wrap_sub(i, begin));
                i64 base = i64(10);
                i64 k = i64(0);
                i64 limit = i64(1844674407370955161);
                i64 remainder = i64(5);
                if ((starts(text, str("0x")) || starts(text, str("0X")))) {
                    base = i64(16);
                    k = i64(2);
                    limit = i64(1152921504606846975);
                    remainder = i64(15);
                }
                bool valid = (k < len(text));
                i64 value = i64(0);
                while ((k < len(text))) {
                    i64 ch = byte(text, k);
                    i64 d = wrap_sub(0, i64(1));
                    if (digit(ch)) {
                        d = wrap_sub(ch, i64(48));
                    } else {
                        if (((ch >= i64(65)) && (ch <= i64(70)))) {
                            d = wrap_sub(ch, i64(55));
                        } else {
                            if (((ch >= i64(97)) && (ch <= i64(102)))) {
                                d = wrap_sub(ch, i64(87));
                            }
                        }
                    }
                    if ((((((d < i64(0)) || (d >= base)) || (value < i64(0))) || (value > limit)) ||
                         ((value == limit) && (d > remainder)))) {
                        valid = false;
                    }
                    value = wrap_add(wrap_mul(value, base), d);
                    k = wrap_add(k, i64(1));
                }
                if ((!valid)) {
                    push(s.tokens, Token{text, i64(2), begin, file});
                    error(s, wrap_sub(len(s.tokens), i64(1)),
                          str("invalid integer literal (expected a 64-bit decimal or hexadecimal "
                              "value)"));
                }
            } else {
                if ((c == i64(34))) {
                    kind = i64(3);
                    i = wrap_add(i, i64(1));
                    Arr<str> chars = {};
                    while (((i < len(src)) && (byte(src, i) != i64(34)))) {
                        i64 ch = byte(src, i);
                        i = wrap_add(i, i64(1));
                        if ((ch == i64(92))) {
                            if ((i == len(src))) {
                                break;
                            }
                            ch = byte(src, i);
                            i = wrap_add(i, i64(1));
                            if ((ch == i64(110))) {
                                ch = i64(10);
                            } else {
                                if ((ch == i64(116))) {
                                    ch = i64(9);
                                } else {
                                    if ((ch == i64(114))) {
                                        ch = i64(13);
                                    } else {
                                        if ((ch == i64(48))) {
                                            ch = i64(0);
                                        } else {
                                            if (((ch != i64(34)) && (ch != i64(92)))) {
                                                push(s.tokens, Token{str(""), i64(0),
                                                                     wrap_sub(i, i64(1)), file});
                                                error(s, wrap_sub(len(s.tokens), i64(1)),
                                                      str("invalid escape"));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        push(chars, chr(ch));
                    }
                    if ((i == len(src))) {
                        push(s.tokens, Token{str(""), i64(0), begin, file});
                        error(s, wrap_sub(len(s.tokens), i64(1)), str("unterminated string"));
                    }
                    i = wrap_add(i, i64(1));
                    text = join(chars, str(""));
                } else {
                    text = chr(c);
                    i = wrap_add(i, i64(1));
                    if ((i < len(src))) {
                        str pair = slice(src, begin, i64(2));
                        if (((((((((((pair == str("->")) || (pair == str("::"))) ||
                                    (pair == str("=="))) ||
                                   (pair == str("!="))) ||
                                  (pair == str("<="))) ||
                                 (pair == str(">="))) ||
                                (pair == str("&&"))) ||
                               (pair == str("||"))) ||
                              (pair == str("<<"))) ||
                             (pair == str(">>")))) {
                            text = pair;
                            i = wrap_add(i, i64(1));
                        }
                    }
                    if ((find({str("("),  str(")"),  str("{"),  str("}"),  str("["),  str("]"),
                               str(","),  str(";"),  str(":"),  str("."),  str("+"),  str("-"),
                               str("*"),  str("/"),  str("%"),  str("&"),  str("|"),  str("^"),
                               str("~"),  str("!"),  str("="),  str("<"),  str(">"),  str("->"),
                               str("::"), str("=="), str("!="), str("<="), str(">="), str("&&"),
                               str("||"), str("<<"), str(">>")},
                              text) < i64(0))) {
                        push(s.tokens, Token{text, i64(4), begin, file});
                        error(s, wrap_sub(len(s.tokens), i64(1)), str("invalid character"));
                    }
                }
            }
        }
        push(s.tokens, Token{text, kind, begin, file});
    }
    push(s.tokens, Token{str("<eof>"), i64(0), len(src), file});
    return first;
}
i64 precedence(str op) {
    if ((op == str("||"))) {
        return i64(1);
    }
    if ((op == str("&&"))) {
        return i64(2);
    }
    if ((op == str("|"))) {
        return i64(3);
    }
    if ((op == str("^"))) {
        return i64(4);
    }
    if ((op == str("&"))) {
        return i64(5);
    }
    if (((op == str("==")) || (op == str("!=")))) {
        return i64(6);
    }
    if (((((op == str("<")) || (op == str(">"))) || (op == str("<="))) || (op == str(">=")))) {
        return i64(7);
    }
    if (((op == str("<<")) || (op == str(">>")))) {
        return i64(8);
    }
    if (((op == str("+")) || (op == str("-")))) {
        return i64(9);
    }
    if ((((op == str("*")) || (op == str("/"))) || (op == str("%")))) {
        return i64(10);
    }
    return wrap_sub(0, i64(1));
}
i64 expression(State s, i64 minp) {
    i64 tok = index(s.cursor, i64(0));
    Token t = index(s.tokens, tok);
    i64 n = wrap_sub(0, i64(1));
    if (eat(s, str("("))) {
        n = expression(s, i64(0));
        need(s, str(")"));
    } else {
        if (eat(s, str("["))) {
            Arr<i64> xs = {};
            if ((peek(s) != str("]"))) {
                push(xs, expression(s, i64(0)));
                while ((eat(s, str(",")) && (peek(s) != str("]")))) {
                    push(xs, expression(s, i64(0)));
                }
            }
            need(s, str("]"));
            n = node(s, str("array"), str(""), str(""), wrap_sub(0, i64(1)), wrap_sub(0, i64(1)),
                     wrap_sub(0, i64(1)), xs, tok);
        } else {
            if (((t.kind == i64(4)) &&
                 ((((t.text == str("!")) || (t.text == str("-"))) || (t.text == str("~")))))) {
                take(s);
                n = node(s, str("unary"), t.text, str(""), expression(s, i64(11)),
                         wrap_sub(0, i64(1)), wrap_sub(0, i64(1)), {}, tok);
            } else {
                if ((t.kind == i64(2))) {
                    take(s);
                    n = node(s, str("int"), t.text, str("i64"), wrap_sub(0, i64(1)),
                             wrap_sub(0, i64(1)), wrap_sub(0, i64(1)), {}, tok);
                } else {
                    if ((t.kind == i64(3))) {
                        take(s);
                        n = node(s, str("string"), t.text, str("str"), wrap_sub(0, i64(1)),
                                 wrap_sub(0, i64(1)), wrap_sub(0, i64(1)), {}, tok);
                    } else {
                        if (((t.text == str("true")) || (t.text == str("false")))) {
                            take(s);
                            n = node(s, str("bool"), t.text, str("bool"), wrap_sub(0, i64(1)),
                                     wrap_sub(0, i64(1)), wrap_sub(0, i64(1)), {}, tok);
                        } else {
                            if ((t.kind == i64(1))) {
                                str text = qualified(s);
                                n = node(s, str("name"), text, str(""), wrap_sub(0, i64(1)),
                                         wrap_sub(0, i64(1)), wrap_sub(0, i64(1)), {}, tok);
                                if ((((byte(text, i64(0)) >= i64(65)) &&
                                      (byte(text, i64(0)) <= i64(90))) &&
                                     eat(s, str("{")))) {
                                    Arr<i64> fields = {};
                                    while ((peek(s) != str("}"))) {
                                        i64 ft = index(s.cursor, i64(0));
                                        str field = name(s);
                                        need(s, str(":"));
                                        i64 value = expression(s, i64(0));
                                        push(fields, node(s, str("init"), field, str(""), value,
                                                          wrap_sub(0, i64(1)), wrap_sub(0, i64(1)),
                                                          {}, ft));
                                        if ((!eat(s, str(",")))) {
                                            break;
                                        }
                                    }
                                    need(s, str("}"));
                                    n = node(s, str("record"), text, text, wrap_sub(0, i64(1)),
                                             wrap_sub(0, i64(1)), wrap_sub(0, i64(1)), fields, tok);
                                }
                            } else {
                                error(s, tok, str("expected expression"));
                            }
                        }
                    }
                }
            }
        }
    }
    while (true) {
        if (eat(s, str("("))) {
            Arr<i64> xs = {};
            if ((peek(s) != str(")"))) {
                push(xs, expression(s, i64(0)));
                while (eat(s, str(","))) {
                    push(xs, expression(s, i64(0)));
                }
            }
            need(s, str(")"));
            n = node(s, str("call"), str(""), str(""), n, wrap_sub(0, i64(1)), wrap_sub(0, i64(1)),
                     xs, tok);
        } else {
            if (eat(s, str("["))) {
                i64 idx = expression(s, i64(0));
                need(s, str("]"));
                n = node(s, str("index"), str(""), str(""), n, idx, wrap_sub(0, i64(1)), {}, tok);
            } else {
                if (eat(s, str("."))) {
                    str field = name(s);
                    n = node(s, str("field"), field, str(""), n, wrap_sub(0, i64(1)),
                             wrap_sub(0, i64(1)), {}, tok);
                } else {
                    if (eat(s, str("as"))) {
                        str ty = parse_type(s);
                        n = node(s, str("cast"), str(""), ty, n, wrap_sub(0, i64(1)),
                                 wrap_sub(0, i64(1)), {}, tok);
                    } else {
                        i64 p = precedence(peek(s));
                        if ((p < minp)) {
                            break;
                        }
                        str op = take(s);
                        i64 rhs = expression(s, wrap_add(p, i64(1)));
                        n = node(s, str("binary"), op, str(""), n, rhs, wrap_sub(0, i64(1)), {},
                                 tok);
                    }
                }
            }
        }
    }
    return n;
}
i64 parse_block(State s) {
    i64 tok = index(s.cursor, i64(0));
    need(s, str("{"));
    Arr<i64> xs = {};
    while (((peek(s) != str("}")) && (peek(s) != str("<eof>")))) {
        push(xs, statement(s));
    }
    need(s, str("}"));
    return node(s, str("block"), str(""), str(""), wrap_sub(0, i64(1)), wrap_sub(0, i64(1)),
                wrap_sub(0, i64(1)), xs, tok);
}
i64 statement(State s) {
    i64 tok = index(s.cursor, i64(0));
    str t = peek(s);
    if ((t == str("{"))) {
        return parse_block(s);
    }
    if (((t == str("let")) || (t == str("var")))) {
        take(s);
        str text = name(s);
        str ty = str("");
        if (eat(s, str(":"))) {
            ty = parse_type(s);
        }
        need(s, str("="));
        i64 value = expression(s, i64(0));
        need(s, str(";"));
        i64 mut = i64(0);
        if ((t == str("var"))) {
            mut = i64(1);
        }
        return node(s, str("decl"), text, ty, value, mut, wrap_sub(0, i64(1)), {}, tok);
    }
    if (eat(s, str("if"))) {
        i64 cond = expression(s, i64(0));
        i64 yes = parse_block(s);
        i64 no = wrap_sub(0, i64(1));
        if (eat(s, str("else"))) {
            if ((peek(s) == str("if"))) {
                no = statement(s);
            } else {
                no = parse_block(s);
            }
        }
        return node(s, str("if"), str(""), str(""), cond, yes, no, {}, tok);
    }
    if (eat(s, str("while"))) {
        i64 cond = expression(s, i64(0));
        i64 body = parse_block(s);
        return node(s, str("while"), str(""), str(""), cond, body, wrap_sub(0, i64(1)), {}, tok);
    }
    if (eat(s, str("return"))) {
        i64 value = wrap_sub(0, i64(1));
        if ((peek(s) != str(";"))) {
            value = expression(s, i64(0));
        }
        need(s, str(";"));
        return node(s, str("return"), str(""), str(""), value, wrap_sub(0, i64(1)),
                    wrap_sub(0, i64(1)), {}, tok);
    }
    if (eat(s, str("break"))) {
        need(s, str(";"));
        return node(s, str("break"), str(""), str(""), wrap_sub(0, i64(1)), wrap_sub(0, i64(1)),
                    wrap_sub(0, i64(1)), {}, tok);
    }
    if (eat(s, str("continue"))) {
        need(s, str(";"));
        return node(s, str("continue"), str(""), str(""), wrap_sub(0, i64(1)), wrap_sub(0, i64(1)),
                    wrap_sub(0, i64(1)), {}, tok);
    }
    i64 lhs = expression(s, i64(0));
    if (eat(s, str("="))) {
        i64 rhs = expression(s, i64(0));
        need(s, str(";"));
        return node(s, str("assign"), str(""), str(""), lhs, rhs, wrap_sub(0, i64(1)), {}, tok);
    }
    need(s, str(";"));
    return node(s, str("expr"), str(""), str(""), lhs, wrap_sub(0, i64(1)), wrap_sub(0, i64(1)), {},
                tok);
}
str module_file(str root, str module) {
    Arr<str> chars = {};
    i64 i = i64(0);
    while ((i < len(module))) {
        if ((byte(module, i) == i64(46))) {
            push(chars, str("/"));
        } else {
            push(chars, chr(byte(module, i)));
        }
        i = wrap_add(i, i64(1));
    }
    return wrap_add(wrap_add(wrap_add(root, str("/")), join(chars, str(""))), str(".aster"));
}
void load_module(State s, str path, str module) {
    if ((find(s.paths, path) >= i64(0))) {
        return;
    }
    i64 file = len(s.paths);
    push(s.paths, path);
    push(s.sources, read_file(path));
    push(s.modules, module);
    i64 saved = index(s.cursor, i64(0));
    index(s.cursor, i64(0)) = lex(s, file);
    while ((peek(s) != str("<eof>"))) {
        i64 tok = index(s.cursor, i64(0));
        if (eat(s, str("import"))) {
            str m = name(s);
            str alias = m;
            while (eat(s, str("."))) {
                alias = name(s);
                m = wrap_add(wrap_add(m, str(".")), alias);
            }
            need(s, str(";"));
            push(s.imports, Import{file, alias, m});
            load_module(s, module_file(s.root, m), m);
        } else {
            if (eat(s, str("struct"))) {
                str text = name(s);
                if (((byte(text, i64(0)) < i64(65)) || (byte(text, i64(0)) > i64(90)))) {
                    error(s, tok, str("record names must begin with an uppercase letter"));
                }
                if ((record_id(s, text) >= i64(0))) {
                    error(s, tok, wrap_add(str("duplicate record "), text));
                }
                need(s, str("{"));
                Arr<str> names = {};
                Arr<str> types = {};
                while ((peek(s) != str("}"))) {
                    str field = name(s);
                    if ((find(names, field) >= i64(0))) {
                        error(s, wrap_sub(index(s.cursor, i64(0)), i64(1)), str("duplicate field"));
                    }
                    push(names, field);
                    need(s, str(":"));
                    push(types, parse_type(s));
                    if ((!eat(s, str(",")))) {
                        break;
                    }
                }
                need(s, str("}"));
                push(s.records, Record{text, names, types, tok});
            } else {
                if (eat(s, str("enum"))) {
                    str text = name(s);
                    if ((record_id(s, text) >= i64(0))) {
                        error(s, tok, wrap_add(str("duplicate type "), text));
                    }
                    need(s, str("{"));
                    Arr<str> names = {};
                    i64 value = i64(0);
                    while ((peek(s) != str("}"))) {
                        str member = name(s);
                        if ((find(names, member) >= i64(0))) {
                            error(s, tok, wrap_add(str("duplicate enum member "), member));
                        }
                        if (eat(s, str("="))) {
                            if ((index(s.tokens, index(s.cursor, i64(0))).kind != i64(2))) {
                                error(s, index(s.cursor, i64(0)),
                                      str("enum value must be an integer literal"));
                            }
                            value = parse_int(take(s));
                        }
                        push(names, member);
                        push(s.functions, Function{wrap_add(wrap_add(text, str("::")), member),
                                                   module,
                                                   {},
                                                   {},
                                                   text,
                                                   node(s, str("enumvalue"), itoa(value), text,
                                                        wrap_sub(0, i64(1)), wrap_sub(0, i64(1)),
                                                        wrap_sub(0, i64(1)), {}, tok),
                                                   tok});
                        value = wrap_add(value, i64(1));
                        if ((!eat(s, str(",")))) {
                            break;
                        }
                    }
                    need(s, str("}"));
                    push(s.records, Record{text, {}, {str("$enum")}, tok});
                } else {
                    if (eat(s, str("fn"))) {
                        str text = name(s);
                        i64 f = i64(0);
                        while ((f < len(s.functions))) {
                            if (((index(s.functions, f).name == text) &&
                                 (index(s.functions, f).module == module))) {
                                error(s, tok, wrap_add(str("duplicate function "), text));
                            }
                            f = wrap_add(f, i64(1));
                        }
                        need(s, str("("));
                        Arr<str> names = {};
                        Arr<str> types = {};
                        while ((peek(s) != str(")"))) {
                            str p = name(s);
                            if ((find(names, p) >= i64(0))) {
                                error(s, tok, str("duplicate parameter"));
                            }
                            push(names, p);
                            need(s, str(":"));
                            push(types, parse_type(s));
                            if ((!eat(s, str(",")))) {
                                break;
                            }
                        }
                        need(s, str(")"));
                        need(s, str("->"));
                        str ret = parse_type(s);
                        i64 body = parse_block(s);
                        push(s.functions, Function{text, module, names, types, ret, body, tok});
                    } else {
                        error(s, tok, str("expected import, struct, enum or fn"));
                    }
                }
            }
        }
    }
    index(s.cursor, i64(0)) = saved;
}
str llvm_type(State s, str t) {
    if (((t == str("i64")) || enum_type(s, t))) {
        return str("i64");
    }
    if ((t == str("u8"))) {
        return str("i8");
    }
    if ((t == str("bool"))) {
        return str("i1");
    }
    if ((t == str("void"))) {
        return str("void");
    }
    if (((t == str("str")) || starts(t, str("[]")))) {
        return str("ptr");
    }
    return wrap_add(str("%R_"), t);
}
str llvm_value(IR g, i64 id) {
    i64 v = canonical(g, id);
    Inst x = index(g.ins, v);
    if ((x.op == str("const"))) {
        return x.text;
    }
    if ((x.op == str("arg"))) {
        return wrap_add(str("%p"), x.text);
    }
    if ((x.op == str("string"))) {
        return wrap_add(wrap_add(wrap_add(str("@ls_"), itoa(g.fid)), str("_")), itoa(v));
    }
    return wrap_add(str("%v"), itoa(v));
}
str typed(IR g, i64 id) {
    i64 v = canonical(g, id);
    return wrap_add(wrap_add(llvm_type(g.s, index(g.ins, v).ty), str(" ")), llvm_value(g, v));
}
str llvm_symbol(State s, i64 fid) {
    return wrap_add(str("@af_"), itoa(fid));
}
str hex_digit(i64 n) {
    return slice(str("0123456789ABCDEF"), n, i64(1));
}
str escaped_bytes(str text) {
    Arr<str> parts = {};
    i64 i = i64(0);
    while ((i < len(text))) {
        i64 c = byte(text, i);
        push(parts, wrap_add(wrap_add(str("\\"), hex_digit(wrap_div(c, i64(16)))),
                             hex_digit(wrap_rem(c, i64(16)))));
        i = wrap_add(i, i64(1));
    }
    return join(parts, str(""));
}
str llvm_literals(IR g) {
    Arr<str> lines = {};
    i64 i = i64(0);
    while ((i < len(g.ins))) {
        Inst x = index(g.ins, i);
        if ((x.op == str("string"))) {
            str suffix = wrap_add(wrap_add(itoa(g.fid), str("_")), itoa(i));
            push(lines, wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(str("@lb_"), suffix),
                                                                     str(" = private constant [")),
                                                            itoa(wrap_add(len(x.text), i64(1)))),
                                                   str(" x i8] c\"")),
                                          escaped_bytes(x.text)),
                                 str("\\00\"\n")));
            push(lines,
                 wrap_add(
                     wrap_add(
                         wrap_add(
                             wrap_add(wrap_add(wrap_add(str("@ls_"), suffix),
                                               str(" = private constant %AsterString { ptr @lb_")),
                                      suffix),
                             str(", i64 ")),
                         itoa(len(x.text))),
                     str(" }\n")));
        }
        i = wrap_add(i, i64(1));
    }
    return join(lines, str(""));
}
str llvm_inst(IR g, i64 id) {
    Inst x = index(g.ins, id);
    str op = x.op;
    str dst = wrap_add(str("%v"), itoa(id));
    str ty = llvm_type(g.s, x.ty);
    str a = str("");
    str b = str("");
    str at = str("");
    if ((x.a >= i64(0))) {
        a = llvm_value(g, x.a);
        at = llvm_type(g.s, index(g.ins, canonical(g, x.a)).ty);
    }
    if ((x.b >= i64(0))) {
        b = llvm_value(g, x.b);
    }
    if ((((op == str("const")) || (op == str("arg"))) || (op == str("string")))) {
        return str("");
    }
    if ((op == str("phi"))) {
        Arr<str> parts = {};
        i64 i = i64(0);
        while ((i < len(x.args))) {
            push(parts,
                 wrap_add(wrap_add(wrap_add(wrap_add(str("[ "), llvm_value(g, index(x.args, i))),
                                            str(", %b")),
                                   itoa(index(x.labels, i))),
                          str(" ]")));
            i = wrap_add(i, i64(1));
        }
        return wrap_add(wrap_add(wrap_add(wrap_add(dst, str(" = phi ")), ty), str(" ")),
                        join(parts, str(", ")));
    }
    if (((((((op == str("add")) || (op == str("sub"))) || (op == str("mul"))) ||
           (op == str("and"))) ||
          (op == str("or"))) ||
         (op == str("xor")))) {
        return wrap_add(
            wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(dst, str(" = ")), op),
                                                         str(" ")),
                                                ty),
                                       str(" ")),
                              a),
                     str(", ")),
            b);
    }
    if (((op == str("shl")) || (op == str("shr")))) {
        str mask = str("63");
        if ((ty == str("i8"))) {
            mask = str("7");
        }
        str instruction = op;
        if ((op == str("shr"))) {
            instruction = str("ashr");
            if ((ty == str("i8"))) {
                instruction = str("lshr");
            }
        }
        return wrap_add(
            wrap_add(
                wrap_add(
                    wrap_add(
                        wrap_add(
                            wrap_add(
                                wrap_add(
                                    wrap_add(
                                        wrap_add(
                                            wrap_add(
                                                wrap_add(
                                                    wrap_add(
                                                        wrap_add(
                                                            wrap_add(
                                                                wrap_add(
                                                                    wrap_add(
                                                                        wrap_add(
                                                                            dst,
                                                                            str("_mask = and ")),
                                                                        ty),
                                                                    str(" ")),
                                                                b),
                                                            str(", ")),
                                                        mask),
                                                    str("\n  ")),
                                                dst),
                                            str(" = ")),
                                        instruction),
                                    str(" ")),
                                ty),
                            str(" ")),
                        a),
                    str(", ")),
                dst),
            str("_mask"));
    }
    if (((op == str("div")) || (op == str("rem")))) {
        if ((ty == str("i64"))) {
            return wrap_add(
                wrap_add(
                    wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(dst, str(" = call i64 @rt_")), op),
                                               str("(i64 ")),
                                      a),
                             str(", i64 ")),
                    b),
                str(")"));
        }
        return wrap_add(
            wrap_add(
                wrap_add(
                    wrap_add(
                        wrap_add(
                            wrap_add(
                                wrap_add(
                                    wrap_add(
                                        wrap_add(
                                            wrap_add(
                                                wrap_add(
                                                    wrap_add(
                                                        wrap_add(
                                                            wrap_add(
                                                                wrap_add(
                                                                    wrap_add(
                                                                        wrap_add(
                                                                            wrap_add(
                                                                                wrap_add(
                                                                                    dst,
                                                                                    str("_a = zext "
                                                                                        "i8 ")),
                                                                                a),
                                                                            str(" to i64\n  ")),
                                                                        dst),
                                                                    str("_b = zext i8 ")),
                                                                b),
                                                            str(" to i64\n  ")),
                                                        dst),
                                                    str("_r = call i64 @rt_")),
                                                op),
                                            str("(i64 ")),
                                        dst),
                                    str("_a, i64 ")),
                                dst),
                            str("_b)\n  ")),
                        dst),
                    str(" = trunc i64 ")),
                dst),
            str("_r to i8"));
    }
    if ((op == str("neg"))) {
        return wrap_add(wrap_add(wrap_add(wrap_add(dst, str(" = sub ")), ty), str(" 0, ")), a);
    }
    if (((op == str("not")) || (op == str("inv")))) {
        return wrap_add(
            wrap_add(wrap_add(wrap_add(wrap_add(dst, str(" = xor ")), ty), str(" ")), a),
            str(", -1"));
    }
    if (((((((op == str("eq")) || (op == str("ne"))) || (op == str("lt"))) || (op == str("le"))) ||
          (op == str("gt"))) ||
         (op == str("ge")))) {
        if ((index(g.ins, canonical(g, x.a)).ty == str("str"))) {
            str cmp = str("ne");
            if ((op == str("ne"))) {
                cmp = str("eq");
            }
            return wrap_add(
                wrap_add(
                    wrap_add(
                        wrap_add(
                            wrap_add(
                                wrap_add(
                                    wrap_add(
                                        wrap_add(
                                            wrap_add(
                                                wrap_add(
                                                    wrap_add(dst,
                                                             str("_cmp = call i64 @rt_equal(ptr ")),
                                                    a),
                                                str(", ptr ")),
                                            b),
                                        str(")\n  ")),
                                    dst),
                                str(" = icmp ")),
                            cmp),
                        str(" i64 ")),
                    dst),
                str("_cmp, 0"));
        }
        str cmp = op;
        if (((op != str("eq")) && (op != str("ne")))) {
            cmp = wrap_add(str("s"), op);
            if ((at == str("i8"))) {
                cmp = wrap_add(str("u"), op);
            }
        }
        return wrap_add(
            wrap_add(
                wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(dst, str(" = icmp ")), cmp),
                                                    str(" ")),
                                           at),
                                  str(" ")),
                         a),
                str(", ")),
            b);
    }
    if ((op == str("cast"))) {
        if ((ty == at)) {
            return wrap_add(
                wrap_add(wrap_add(wrap_add(wrap_add(dst, str(" = add ")), ty), str(" ")), a),
                str(", 0"));
        }
        if ((ty == str("i1"))) {
            return wrap_add(
                wrap_add(wrap_add(wrap_add(wrap_add(dst, str(" = icmp ne ")), at), str(" ")), a),
                str(", 0"));
        }
        str instruction = str("zext");
        if ((at == str("i64"))) {
            instruction = str("trunc");
        }
        return wrap_add(
            wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(dst, str(" = ")),
                                                                  instruction),
                                                         str(" ")),
                                                at),
                                       str(" ")),
                              a),
                     str(" to ")),
            ty);
    }
    if ((op == str("concat"))) {
        return wrap_add(
            wrap_add(wrap_add(wrap_add(wrap_add(dst, str(" = call ptr @rt_concat(ptr ")), a),
                              str(", ptr ")),
                     b),
            str(")"));
    }
    if ((op == str("len"))) {
        return wrap_add(wrap_add(wrap_add(dst, str(" = call i64 @rt_len(ptr ")), a), str(")"));
    }
    if ((op == str("byte"))) {
        return wrap_add(
            wrap_add(
                wrap_add(
                    wrap_add(
                        wrap_add(
                            wrap_add(
                                wrap_add(
                                    wrap_add(wrap_add(dst, str("_n = call i64 @rt_byte(ptr ")), a),
                                    str(", i64 ")),
                                b),
                            str(")\n  ")),
                        dst),
                    str(" = trunc i64 ")),
                dst),
            str("_n to i8"));
    }
    if ((op == str("array"))) {
        str et = llvm_type(g.s, x.text);
        return wrap_add(
            wrap_add(wrap_add(dst, str(" = call ptr @rt_array(i64 ptrtoint (ptr getelementptr (")),
                     et),
            str(", ptr null, i32 1) to i64))"));
    }
    if ((op == str("get"))) {
        return wrap_add(
            wrap_add(
                wrap_add(
                    wrap_add(
                        wrap_add(
                            wrap_add(
                                wrap_add(
                                    wrap_add(
                                        wrap_add(
                                            wrap_add(
                                                wrap_add(dst, str("_ptr = call ptr @rt_at(ptr ")),
                                                a),
                                            str(", i64 ")),
                                        b),
                                    str(")\n  ")),
                                dst),
                            str(" = load ")),
                        ty),
                    str(", ptr ")),
                dst),
            str("_ptr"));
    }
    if ((op == str("set"))) {
        return wrap_add(
            wrap_add(
                wrap_add(
                    wrap_add(
                        wrap_add(
                            wrap_add(
                                wrap_add(
                                    wrap_add(wrap_add(dst, str("_ptr = call ptr @rt_at(ptr ")), a),
                                    str(", i64 ")),
                                b),
                            str(")\n  store ")),
                        typed(g, index(x.args, i64(0)))),
                    str(", ptr ")),
                dst),
            str("_ptr"));
    }
    if ((op == str("push"))) {
        return wrap_add(
            wrap_add(
                wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(str("store "), typed(g, x.b)),
                                                             str(", ptr %slot")),
                                                    itoa(id)),
                                           str("\n  call void @rt_push(ptr ")),
                                  a),
                         str(", ptr %slot")),
                itoa(id)),
            str(")"));
    }
    if ((op == str("pop"))) {
        return wrap_add(wrap_add(str("call void @rt_pop(ptr "), a), str(")"));
    }
    if ((op == str("extract"))) {
        return wrap_add(
            wrap_add(wrap_add(wrap_add(dst, str(" = extractvalue ")), typed(g, x.a)), str(", ")),
            x.text);
    }
    if ((op == str("insert"))) {
        return wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(dst, str(" = insertvalue ")),
                                                            typed(g, x.a)),
                                                   str(", ")),
                                          typed(g, x.b)),
                                 str(", ")),
                        x.text);
    }
    if ((op == str("record"))) {
        if ((len(x.args) == i64(0))) {
            return wrap_add(wrap_add(wrap_add(dst, str(" = freeze ")), ty),
                            str(" zeroinitializer"));
        }
        Arr<str> parts = {};
        i64 i = i64(0);
        while ((i < len(x.args))) {
            str prev = str("zeroinitializer");
            if ((i > i64(0))) {
                prev = wrap_add(wrap_add(dst, str("_f")), itoa(wrap_sub(i, i64(1))));
            }
            str name = wrap_add(wrap_add(dst, str("_f")), itoa(i));
            if ((i == wrap_sub(len(x.args), i64(1)))) {
                name = dst;
            }
            push(parts,
                 wrap_add(
                     wrap_add(
                         wrap_add(
                             wrap_add(
                                 wrap_add(
                                     wrap_add(wrap_add(wrap_add(name, str(" = insertvalue ")), ty),
                                              str(" ")),
                                     prev),
                                 str(", ")),
                             typed(g, index(x.args, i))),
                         str(", ")),
                     itoa(i)));
            i = wrap_add(i, i64(1));
        }
        return join(parts, str("\n  "));
    }
    if ((op == str("call"))) {
        str target = wrap_add(str("@rt_"), x.text);
        if (starts(x.text, str("@"))) {
            target =
                llvm_symbol(g.s, parse_int(slice(x.text, i64(1), wrap_sub(len(x.text), i64(1)))));
        }
        Arr<str> parts = {};
        i64 i = i64(0);
        while ((i < len(x.args))) {
            push(parts, typed(g, index(x.args, i)));
            i = wrap_add(i, i64(1));
        }
        str prefix = wrap_add(dst, str(" = "));
        if ((ty == str("void"))) {
            prefix = str("");
        }
        return wrap_add(
            wrap_add(
                wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(prefix, str("call ")), ty), str(" ")),
                                  target),
                         str("(")),
                join(parts, str(", "))),
            str(")"));
    }
    error(g.s, index(g.s.functions, g.fid).tok, wrap_add(str("LLVM emitter missing opcode "), op));
    return str("");
}
str llvm_function(IR g) {
    Function f = index(g.s.functions, g.fid);
    Arr<str> lines = {};
    Arr<str> params = {};
    i64 i = i64(0);
    while ((i < len(f.types))) {
        push(params, wrap_add(wrap_add(llvm_type(g.s, index(f.types, i)), str(" %p")), itoa(i)));
        i = wrap_add(i, i64(1));
    }
    push(lines,
         wrap_add(
             wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(str("define "), llvm_type(g.s, f.ret)),
                                                 str(" ")),
                                        llvm_symbol(g.s, g.fid)),
                               str("(")),
                      join(params, str(", "))),
             str(") {\n")));
    i64 b = i64(0);
    while ((b < len(g.blocks))) {
        if (index(g.blocks, b).alive) {
            push(lines, wrap_add(wrap_add(str("b"), itoa(b)), str(":\n")));
            if ((b == i64(0))) {
                i = i64(0);
                while ((i < len(g.ins))) {
                    Inst x = index(g.ins, i);
                    if ((((x.op == str("push")) && (!x.dead)) && (x.repl < i64(0)))) {
                        push(lines, wrap_add(wrap_add(wrap_add(wrap_add(str("  %slot"), itoa(i)),
                                                               str(" = alloca ")),
                                                      llvm_type(g.s, x.text)),
                                             str("\n")));
                    }
                    i = wrap_add(i, i64(1));
                }
            }
            i64 phase = i64(0);
            while ((phase < i64(2))) {
                i = i64(0);
                while ((i < len(index(g.blocks, b).code))) {
                    i64 id = index(index(g.blocks, b).code, i);
                    Inst x = index(g.ins, id);
                    if (((((!x.dead) && (x.repl < i64(0))) && (x.block == b)) &&
                         (((((phase == i64(0)) && (x.op == str("phi")))) ||
                           (((phase == i64(1)) && (x.op != str("phi")))))))) {
                        str line = llvm_inst(g, id);
                        if ((line != str(""))) {
                            push(lines, wrap_add(wrap_add(str("  "), line), str("\n")));
                        }
                    }
                    i = wrap_add(i, i64(1));
                }
                phase = wrap_add(phase, i64(1));
            }
            Block block = index(g.blocks, b);
            if ((block.term == str("return"))) {
                if ((len(block.values) == i64(0))) {
                    push(lines, str("  ret void\n"));
                } else {
                    push(lines,
                         wrap_add(wrap_add(str("  ret "), typed(g, index(block.values, i64(0)))),
                                  str("\n")));
                }
            } else {
                if ((block.term == str("jump"))) {
                    push(lines, wrap_add(wrap_add(str("  br label %b"),
                                                  itoa(index(block.targets, i64(0)))),
                                         str("\n")));
                } else {
                    if ((block.term == str("branch"))) {
                        push(
                            lines,
                            wrap_add(
                                wrap_add(
                                    wrap_add(
                                        wrap_add(wrap_add(wrap_add(str("  br i1 "),
                                                                   llvm_value(g, index(block.values,
                                                                                       i64(0)))),
                                                          str(", label %b")),
                                                 itoa(index(block.targets, i64(0)))),
                                        str(", label %b")),
                                    itoa(index(block.targets, i64(1)))),
                                str("\n")));
                    } else {
                        error(g.s, f.tok, str("unterminated IR block"));
                    }
                }
            }
        }
        b = wrap_add(b, i64(1));
    }
    push(lines, str("}\n"));
    return join(lines, str(""));
}
str llvm_prelude(State s) {
    Arr<str> lines = {
        str("; AsterCC deterministic LLVM backend\nsource_filename = \"aster\"\ntarget triple = "
            "\"x86_64-pc-linux-gnu\"\n%AsterString = type { ptr, i64 }\n")};
    i64 i = i64(0);
    while ((i < len(s.records))) {
        Record r = index(s.records, i);
        if ((!enum_type(s, r.name))) {
            Arr<str> fields = {};
            i64 j = i64(0);
            while ((j < len(r.types))) {
                push(fields, llvm_type(s, index(r.types, j)));
                j = wrap_add(j, i64(1));
            }
            push(lines, wrap_add(wrap_add(wrap_add(wrap_add(str("%R_"), r.name), str(" = type { ")),
                                          join(fields, str(", "))),
                                 str(" }\n")));
        }
        i = wrap_add(i, i64(1));
    }
    push(lines,
         str("declare i64 @rt_len(ptr)\ndeclare ptr @rt_array(i64)\ndeclare ptr @rt_at(ptr, "
             "i64)\ndeclare void @rt_push(ptr, ptr)\ndeclare void @rt_pop(ptr)\ndeclare i64 "
             "@rt_byte(ptr, i64)\ndeclare ptr @rt_concat(ptr, ptr)\ndeclare i64 @rt_equal(ptr, "
             "ptr)\ndeclare i64 @rt_div(i64, i64)\ndeclare i64 @rt_rem(i64, i64)\n"));
    Arr<str> names = {str("print"),       str("eprint"), str("println"),   str("print_int"),
                      str("println_int"), str("exit"),   str("read_file"), str("write_file"),
                      str("env"),         str("itoa"),   str("chr"),       str("parse_int"),
                      str("slice"),       str("join"),   str("args"),      str("run"),
                      str("clock_ns")};
    i = i64(0);
    while ((i < len(names))) {
        Arr<str> types = builtin_types(index(names, i));
        Arr<str> args = {};
        i64 j = i64(0);
        while ((j < len(types))) {
            push(args, llvm_type(s, index(types, j)));
            j = wrap_add(j, i64(1));
        }
        push(lines,
             wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(str("declare "),
                                                                   llvm_type(s, builtin_ret(index(
                                                                                    names, i)))),
                                                          str(" @rt_")),
                                                 index(names, i)),
                                        str("(")),
                               join(args, str(", "))),
                      str(")\n")));
        i = wrap_add(i, i64(1));
    }
    return join(lines, str(""));
}
str json_quote(str s) {
    Arr<str> chars = {str("\"")};
    i64 i = i64(0);
    while ((i < len(s))) {
        i64 c = byte(s, i);
        if ((c == i64(34))) {
            push(chars, str("\\\""));
        } else {
            if ((c == i64(92))) {
                push(chars, str("\\\\"));
            } else {
                if ((c < i64(32))) {
                    push(chars, wrap_add(wrap_add(str("\\u00"), hex_digit(wrap_div(c, i64(16)))),
                                         hex_digit(wrap_rem(c, i64(16)))));
                } else {
                    push(chars, chr(c));
                }
            }
        }
        i = wrap_add(i, i64(1));
    }
    push(chars, str("\""));
    return join(chars, str(""));
}
Options parse_options(Arr<str> argv) {
    Options o = Options{str(""), str(""), str("exe"), i64(0),  wrap_sub(0, i64(1)),
                        str(""), false,   str(""),    str(""), false};
    i64 i = i64(1);
    while ((i < len(argv))) {
        str a = index(argv, i);
        if ((a == str("--help"))) {
            println(str("AsterCC 0.1.0\nUsage: astercc source.aster [-o output] [-O0|-O1|-O2]\n  "
                        "--check --emit=ast|ssa|llvm|obj|exe --llvm-opt=0|1|2\n  "
                        "--passes=simplify,sccp,cfg,dce,gvn,licm --verify-ir\n  --stats-json PATH "
                        "--trace-dir DIRECTORY"));
            exit(i64(0));
        } else {
            if ((a == str("--version"))) {
                println(str("AsterCC 0.1.0 (Aster SSA / LLVM 18)"));
                exit(i64(0));
            } else {
                if ((((a == str("-o")) || (a == str("--stats-json"))) ||
                     (a == str("--trace-dir")))) {
                    i = wrap_add(i, i64(1));
                    if ((i >= len(argv))) {
                        eprint(wrap_add(wrap_add(str("missing value for "), a), str("\n")));
                        exit(i64(2));
                    }
                    if ((a == str("-o"))) {
                        o.output = index(argv, i);
                    } else {
                        if ((a == str("--stats-json"))) {
                            o.stats = index(argv, i);
                        } else {
                            o.trace = index(argv, i);
                        }
                    }
                } else {
                    if ((((a == str("-O0")) || (a == str("-O1"))) || (a == str("-O2")))) {
                        o.opt = parse_int(slice(a, i64(2), i64(1)));
                    } else {
                        if (starts(a, str("--emit="))) {
                            o.emit = slice(a, i64(7), wrap_sub(len(a), i64(7)));
                        } else {
                            if (starts(a, str("--llvm-opt="))) {
                                o.llvm = parse_int(slice(a, i64(11), wrap_sub(len(a), i64(11))));
                            } else {
                                if (starts(a, str("--passes="))) {
                                    o.passes = slice(a, i64(9), wrap_sub(len(a), i64(9)));
                                } else {
                                    if ((a == str("--verify-ir"))) {
                                        o.verify = true;
                                    } else {
                                        if ((a == str("--check"))) {
                                            o.check = true;
                                        } else {
                                            if (starts(a, str("-"))) {
                                                eprint(wrap_add(wrap_add(str("unknown option "), a),
                                                                str("\n")));
                                                exit(i64(2));
                                            } else {
                                                if ((o.input != str(""))) {
                                                    eprint(str(
                                                        "exactly one entry module is required\n"));
                                                    exit(i64(2));
                                                }
                                                o.input = a;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        i = wrap_add(i, i64(1));
    }
    if ((o.input == str(""))) {
        eprint(str("usage: astercc source.aster [-o output] [-O0|-O1|-O2]\n"));
        exit(i64(2));
    }
    if ((find({str("ast"), str("ssa"), str("llvm"), str("obj"), str("exe")}, o.emit) < i64(0))) {
        eprint(str("invalid emission kind\n"));
        exit(i64(2));
    }
    if ((o.llvm < i64(0))) {
        o.llvm = o.opt;
    }
    if ((o.llvm > i64(2))) {
        eprint(str("LLVM optimization level must be 0, 1 or 2\n"));
        exit(i64(2));
    }
    if ((o.output == str(""))) {
        o.output = str("a.out");
        if ((o.emit == str("llvm"))) {
            o.output = str("a.ll");
        }
        if ((o.emit == str("ssa"))) {
            o.output = str("a.ssa");
        }
        if ((o.emit == str("ast"))) {
            o.output = str("a.ast.json");
        }
        if ((o.emit == str("obj"))) {
            o.output = str("a.o");
        }
    }
    return o;
}
Arr<str> split_commas(str s) {
    Arr<str> xs = {};
    i64 begin = i64(0);
    i64 i = i64(0);
    while ((i <= len(s))) {
        if (((i == len(s)) || (byte(s, i) == i64(44)))) {
            push(xs, slice(s, begin, wrap_sub(i, begin)));
            begin = wrap_add(i, i64(1));
        }
        i = wrap_add(i, i64(1));
    }
    return xs;
}
str pass_requires(str p) {
    if (((p == str("gvn")) || (p == str("licm")))) {
        return str("dominators");
    }
    if (((p == str("sccp")) || (p == str("dce")))) {
        return str("use-def");
    }
    return str("none");
}
str pass_invalidates(str p) {
    if (((p == str("sccp")) || (p == str("cfg")))) {
        return str("cfg,dominators,loops,use-def");
    }
    return str("use-def");
}
void optimize(IR g, Options o, Arr<str> stats) {
    Arr<str> passes = {};
    if ((o.opt >= i64(1))) {
        passes = {str("simplify"), str("sccp"), str("cfg"), str("dce")};
    }
    if ((o.opt == i64(2))) {
        passes = {str("simplify"), str("sccp"),     str("cfg"),  str("dce"), str("gvn"),
                  str("licm"),     str("simplify"), str("sccp"), str("cfg"), str("dce")};
    }
    if ((o.passes != str(""))) {
        passes = split_commas(o.passes);
    }
    if (o.verify) {
        verify_ir(g);
    }
    if ((o.trace != str(""))) {
        write_file(
            wrap_add(wrap_add(wrap_add(o.trace, str("/")), itoa(g.fid)), str("-00-input.ssa")),
            dump_ir(g));
    }
    i64 i = i64(0);
    while ((i < len(passes))) {
        str p = index(passes, i);
        i64 before = count_ir(g);
        Arr<str> states = {};
        i64 j = i64(0);
        if ((o.stats != str(""))) {
            while ((j < len(g.ins))) {
                Inst x = index(g.ins, j);
                push(states,
                     wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(instruction_key(g, j),
                                                                           str(":")),
                                                                  itoa(x.block)),
                                                         str(":")),
                                                itoa(x.repl)),
                                       str(":")),
                              itoa(static_cast<i64>(x.dead))));
                j = wrap_add(j, i64(1));
            }
        }
        i64 begin = clock_ns();
        if ((p == str("simplify"))) {
            simplify(g);
        } else {
            if ((p == str("sccp"))) {
                sccp(g);
            } else {
                if ((p == str("cfg"))) {
                    cfg_simplify(g);
                } else {
                    if ((p == str("dce"))) {
                        dce(g);
                    } else {
                        if ((p == str("gvn"))) {
                            gvn(g);
                        } else {
                            if ((p == str("licm"))) {
                                licm(g);
                            } else {
                                error(g.s, index(g.s.functions, g.fid).tok,
                                      wrap_add(str("unknown pass "), p));
                            }
                        }
                    }
                }
            }
        }
        i64 elapsed = wrap_sub(clock_ns(), begin);
        if (o.verify) {
            verify_ir(g);
        }
        if ((o.stats != str(""))) {
            i64 changes = i64(0);
            j = i64(0);
            while ((j < len(g.ins))) {
                Inst x = index(g.ins, j);
                if ((index(states, j) !=
                     wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(instruction_key(g, j),
                                                                           str(":")),
                                                                  itoa(x.block)),
                                                         str(":")),
                                                itoa(x.repl)),
                                       str(":")),
                              itoa(static_cast<i64>(x.dead))))) {
                    changes = wrap_add(changes, i64(1));
                }
                j = wrap_add(j, i64(1));
            }
            push(
                stats,
                wrap_add(
                    wrap_add(
                        wrap_add(
                            wrap_add(
                                wrap_add(
                                    wrap_add(
                                        wrap_add(
                                            wrap_add(
                                                wrap_add(
                                                    wrap_add(
                                                        wrap_add(
                                                            wrap_add(
                                                                wrap_add(
                                                                    wrap_add(
                                                                        wrap_add(
                                                                            wrap_add(
                                                                                str("{\"function\""
                                                                                    ":"),
                                                                                json_quote(
                                                                                    index(
                                                                                        g.s.functions,
                                                                                        g.fid)
                                                                                        .name)),
                                                                            str(",\"pass\":")),
                                                                        json_quote(p)),
                                                                    str(",\"requires\":")),
                                                                json_quote(pass_requires(p))),
                                                            str(",\"invalidates\":")),
                                                        json_quote(pass_invalidates(p))),
                                                    str(",\"changed_instructions\":")),
                                                itoa(changes)),
                                            str(",\"before\":")),
                                        itoa(before)),
                                    str(",\"after\":")),
                                itoa(count_ir(g))),
                            str(",\"ns\":")),
                        itoa(elapsed)),
                    str("}")));
        }
        if ((o.trace != str(""))) {
            write_file(
                wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(o.trace, str("/")),
                                                                      itoa(g.fid)),
                                                             str("-")),
                                                    itoa(wrap_add(i, i64(1)))),
                                           str("-")),
                                  p),
                         str(".ssa")),
                dump_ir(g));
        }
        i = wrap_add(i, i64(1));
    }
}
str dump_ast(State s) {
    Arr<str> nodes = {};
    Arr<str> functions = {};
    Arr<str> records = {};
    i64 i = i64(0);
    while ((i < len(s.nodes))) {
        Node n = index(s.nodes, i);
        Arr<str> kids = {};
        i64 j = i64(0);
        while ((j < len(n.kids))) {
            push(kids, itoa(index(n.kids, j)));
            j = wrap_add(j, i64(1));
        }
        push(
            nodes,
            wrap_add(
                wrap_add(
                    wrap_add(
                        wrap_add(
                            wrap_add(
                                wrap_add(
                                    wrap_add(
                                        wrap_add(
                                            wrap_add(
                                                wrap_add(
                                                    wrap_add(
                                                        wrap_add(
                                                            wrap_add(
                                                                wrap_add(
                                                                    wrap_add(
                                                                        wrap_add(
                                                                            wrap_add(
                                                                                wrap_add(
                                                                                    wrap_add(
                                                                                        wrap_add(
                                                                                            str("{"
                                                                                                "\""
                                                                                                "id"
                                                                                                "\""
                                                                                                ":"),
                                                                                            itoa(
                                                                                                i)),
                                                                                        str(",\"tag"
                                                                                            "\":")),
                                                                                    json_quote(
                                                                                        n.tag)),
                                                                                str(",\"text\":")),
                                                                            json_quote(n.text)),
                                                                        str(",\"type\":")),
                                                                    json_quote(n.ty)),
                                                                str(",\"a\":")),
                                                            itoa(n.a)),
                                                        str(",\"b\":")),
                                                    itoa(n.b)),
                                                str(",\"c\":")),
                                            itoa(n.c)),
                                        str(",\"children\":[")),
                                    join(kids, str(","))),
                                str("],\"file\":")),
                            itoa(index(s.tokens, n.tok).file)),
                        str(",\"offset\":")),
                    itoa(index(s.tokens, n.tok).pos)),
                str("}")));
        i = wrap_add(i, i64(1));
    }
    i = i64(0);
    while ((i < len(s.functions))) {
        Function f = index(s.functions, i);
        Arr<str> params = {};
        i64 j = i64(0);
        while ((j < len(f.names))) {
            push(params, wrap_add(wrap_add(wrap_add(wrap_add(str("{\"name\":"),
                                                             json_quote(index(f.names, j))),
                                                    str(",\"type\":")),
                                           json_quote(index(f.types, j))),
                                  str("}")));
            j = wrap_add(j, i64(1));
        }
        push(functions,
             wrap_add(
                 wrap_add(
                     wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(str("{\"name\":"),
                                                                           json_quote(wrap_add(
                                                                               wrap_add(f.module,
                                                                                        str("::")),
                                                                               f.name))),
                                                                  str(",\"parameters\":[")),
                                                         join(params, str(","))),
                                                str("],\"return\":")),
                                       json_quote(f.ret)),
                              str(",\"body\":")),
                     itoa(f.body)),
                 str("}")));
        i = wrap_add(i, i64(1));
    }
    i = i64(0);
    while ((i < len(s.records))) {
        Record r = index(s.records, i);
        Arr<str> fields = {};
        i64 j = i64(0);
        while ((j < len(r.names))) {
            push(fields, wrap_add(wrap_add(wrap_add(wrap_add(str("{\"name\":"),
                                                             json_quote(index(r.names, j))),
                                                    str(",\"type\":")),
                                           json_quote(index(r.types, j))),
                                  str("}")));
            j = wrap_add(j, i64(1));
        }
        push(records, wrap_add(wrap_add(wrap_add(wrap_add(str("{\"name\":"), json_quote(r.name)),
                                                 str(",\"fields\":[")),
                                        join(fields, str(","))),
                               str("]}")));
        i = wrap_add(i, i64(1));
    }
    return wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(str("{\"records\":["),
                                                                 join(records, str(","))),
                                                        str("],\"functions\":[")),
                                               join(functions, str(","))),
                                      str("],\"nodes\":[\n")),
                             join(nodes, str(",\n"))),
                    str("\n]}\n"));
}
i64 aster_main() {
    Options o = parse_options(args());
    str root = env(str("ASTERCC_ROOT"));
    if ((root == str(""))) {
        root = str(".");
    }
    State s = new_state(root);
    load_module(s, o.input, str("main"));
    i64 r = i64(0);
    while ((r < len(s.records))) {
        check_layout(s, index(s.records, r).name, {}, index(s.records, r).tok);
        r = wrap_add(r, i64(1));
    }
    i64 entry = wrap_sub(0, i64(1));
    i64 i = i64(0);
    while ((i < len(s.functions))) {
        Function f = index(s.functions, i);
        map_put(s.symbols, wrap_add(wrap_add(f.module, str("::")), f.name), i);
        i64 old = map_get(s.symbols, wrap_add(str("*::"), f.name));
        if ((old == wrap_sub(0, i64(1)))) {
            map_put(s.symbols, wrap_add(str("*::"), f.name), i);
        } else {
            map_put(s.symbols, wrap_add(str("*::"), f.name), wrap_sub(0, i64(2)));
        }
        if (((f.name == str("main")) && (f.module == str("main")))) {
            entry = i;
        }
        i = wrap_add(i, i64(1));
    }
    if ((entry < i64(0))) {
        eprint(str("error: entry module needs fn main() -> i64\n"));
        return i64(1);
    }
    if (((len(index(s.functions, entry).types) != i64(0)) ||
         (index(s.functions, entry).ret != str("i64")))) {
        error(s, index(s.functions, entry).tok, str("main must have type fn() -> i64"));
    }
    if ((o.trace != str(""))) {
        if ((run({str("mkdir"), str("-p"), o.trace}) != i64(0))) {
            return i64(1);
        }
    }
    Arr<str> globals = {};
    Arr<str> functions = {};
    Arr<str> ssa = {};
    Arr<str> stats = {};
    i = i64(0);
    while ((i < len(s.functions))) {
        if ((index(s.nodes, index(s.functions, i).body).tag != str("enumvalue"))) {
            IR g = lower_function(s, i);
            optimize(g, o, stats);
            if ((!o.check)) {
                push(ssa, dump_ir(g));
                if (((o.emit != str("ssa")) && (o.emit != str("ast")))) {
                    push(globals, llvm_literals(g));
                    push(functions, llvm_function(g));
                }
            }
        }
        i = wrap_add(i, i64(1));
    }
    if ((o.stats != str(""))) {
        write_file(o.stats, wrap_add(wrap_add(str("[\n"), join(stats, str(",\n"))), str("\n]\n")));
    }
    if (o.check) {
        return i64(0);
    }
    if ((o.emit == str("ssa"))) {
        return write_file(o.output, join(ssa, str("\n")));
    }
    if ((o.emit == str("ast"))) {
        return write_file(o.output, dump_ast(s));
    }
    str llvm = wrap_add(
        wrap_add(wrap_add(wrap_add(wrap_add(llvm_prelude(s), join(globals, str(""))),
                                   join(functions, str("\n"))),
                          str("\ndefine i64 @aster_entry() {\nentry:\n  %result = call i64 ")),
                 llvm_symbol(s, entry)),
        str("()\n  ret i64 %result\n}\n"));
    if ((o.emit == str("llvm"))) {
        return write_file(o.output, llvm);
    }
    str intermediate = wrap_add(o.output, str(".ll"));
    if ((write_file(intermediate, llvm) != i64(0))) {
        return i64(1);
    }
    str clang = env(str("ASTERCC_CLANG"));
    if ((clang == str(""))) {
        clang = wrap_add(root, str("/.toolchain/usr/lib/llvm-18/bin/clang"));
    }
    Arr<str> command = {clang,
                        str("-Wno-override-module"),
                        wrap_add(str("-O"), itoa(o.llvm)),
                        str("-x"),
                        str("ir"),
                        intermediate};
    if ((o.emit == str("obj"))) {
        push(command, str("-c"));
    } else {
        push(command, str("-x"));
        push(command, str("none"));
        push(command, wrap_add(root, str("/build/libasterrt.a")));
        push(command, str("-Wl,--build-id=none"));
    }
    push(command, str("-o"));
    push(command, o.output);
    return run(command);
}
i64 byte(str s, i64 i) {
    return static_cast<i64>(index(s, i));
}
bool starts(str s, str prefix) {
    return ((len(s) >= len(prefix)) && (slice(s, i64(0), len(prefix)) == prefix));
}
bool ends(str s, str suffix) {
    return ((len(s) >= len(suffix)) &&
            (slice(s, wrap_sub(len(s), len(suffix)), len(suffix)) == suffix));
}
bool has(Arr<i64> xs, i64 x) {
    i64 i = i64(0);
    while ((i < len(xs))) {
        if ((index(xs, i) == x)) {
            return true;
        }
        i = wrap_add(i, i64(1));
    }
    return false;
}
i64 find(Arr<str> xs, str x) {
    i64 i = i64(0);
    while ((i < len(xs))) {
        if ((index(xs, i) == x)) {
            return i;
        }
        i = wrap_add(i, i64(1));
    }
    return wrap_sub(0, i64(1));
}
Arr<i64> copy_ids(Arr<i64> xs) {
    Arr<i64> r = {};
    i64 i = i64(0);
    while ((i < len(xs))) {
        push(r, index(xs, i));
        i = wrap_add(i, i64(1));
    }
    return r;
}
State new_state(str root) {
    return State{{}, {}, {}, {}, {}, {}, {}, {}, {i64(0)}, root, map_new()};
}
void error(State s, i64 tok, str message) {
    Token t = index(s.tokens, tok);
    str src = index(s.sources, t.file);
    i64 line = i64(1);
    i64 col = i64(1);
    i64 begin = i64(0);
    i64 i = i64(0);
    while ((i < t.pos)) {
        if ((byte(src, i) == i64(10))) {
            line = wrap_add(line, i64(1));
            col = i64(1);
            begin = wrap_add(i, i64(1));
        } else {
            col = wrap_add(col, i64(1));
        }
        i = wrap_add(i, i64(1));
    }
    i64 stop = t.pos;
    while (((stop < len(src)) && (byte(src, stop) != i64(10)))) {
        stop = wrap_add(stop, i64(1));
    }
    eprint(wrap_add(
        wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(index(s.paths, t.file), str(":")),
                                                     itoa(line)),
                                            str(":")),
                                   itoa(col)),
                          str(": error: ")),
                 message),
        str("\n")));
    eprint(wrap_add(slice(src, begin, wrap_sub(stop, begin)), str("\n")));
    Arr<str> spaces = {};
    i = i64(1);
    while ((i < col)) {
        push(spaces, str(" "));
        i = wrap_add(i, i64(1));
    }
    eprint(wrap_add(join(spaces, str("")), str("^\n")));
    exit(i64(1));
}
i64 node(State s, str tag, str text, str ty, i64 a, i64 b, i64 c, Arr<i64> kids, i64 tok) {
    i64 n = len(s.nodes);
    push(s.nodes, Node{tag, text, ty, a, b, c, kids, tok});
    return n;
}
str peek(State s) {
    if ((index(s.tokens, index(s.cursor, i64(0))).kind == i64(3))) {
        return str("<string>");
    }
    return index(s.tokens, index(s.cursor, i64(0))).text;
}
str take(State s) {
    str t = index(s.tokens, index(s.cursor, i64(0))).text;
    index(s.cursor, i64(0)) = wrap_add(index(s.cursor, i64(0)), i64(1));
    return t;
}
bool eat(State s, str t) {
    if ((peek(s) == t)) {
        index(s.cursor, i64(0)) = wrap_add(index(s.cursor, i64(0)), i64(1));
        return true;
    }
    return false;
}
void need(State s, str t) {
    if ((!eat(s, t))) {
        error(s, index(s.cursor, i64(0)),
              wrap_add(wrap_add(wrap_add(wrap_add(str("expected '"), t), str("', got '")), peek(s)),
                       str("'")));
    }
}
str name(State s) {
    if ((index(s.tokens, index(s.cursor, i64(0))).kind != i64(1))) {
        error(s, index(s.cursor, i64(0)), str("expected identifier"));
    }
    return take(s);
}
str qualified(State s) {
    str n = name(s);
    while (eat(s, str("::"))) {
        n = wrap_add(wrap_add(n, str("::")), name(s));
    }
    return n;
}
str parse_type(State s) {
    if (eat(s, str("["))) {
        need(s, str("]"));
        return wrap_add(str("[]"), parse_type(s));
    }
    return qualified(s);
}
i64 record_id(State s, str n) {
    i64 i = i64(0);
    while ((i < len(s.records))) {
        if ((index(s.records, i).name == n)) {
            return i;
        }
        i = wrap_add(i, i64(1));
    }
    return wrap_sub(0, i64(1));
}
str element(str t) {
    return slice(t, i64(2), wrap_sub(len(t), i64(2)));
}
bool is_integer(str t) {
    return ((t == str("i64")) || (t == str("u8")));
}
bool known_type(State s, str t) {
    if (starts(t, str("[]"))) {
        return ((element(t) != str("void")) && known_type(s, element(t)));
    }
    return (
        (((((t == str("i64")) || (t == str("u8"))) || (t == str("str"))) || (t == str("bool"))) ||
         (t == str("void"))) ||
        (record_id(s, t) >= i64(0)));
}
bool pure(str op) {
    return (find({str("const"), str("string"), str("arg"),     str("add"),   str("sub"),
                  str("mul"),   str("shl"),    str("shr"),     str("and"),   str("or"),
                  str("xor"),   str("neg"),    str("inv"),     str("not"),   str("eq"),
                  str("ne"),    str("lt"),     str("le"),      str("gt"),    str("ge"),
                  str("cast"),  str("record"), str("extract"), str("insert")},
                 op) >= i64(0));
}
Arr<i64> operands(Inst x) {
    Arr<i64> xs = {};
    if ((x.a >= i64(0))) {
        push(xs, x.a);
    }
    if ((x.b >= i64(0))) {
        push(xs, x.b);
    }
    i64 i = i64(0);
    while ((i < len(x.args))) {
        push(xs, index(x.args, i));
        i = wrap_add(i, i64(1));
    }
    return xs;
}
bool live_inst(IR g, i64 id) {
    Inst x = index(g.ins, id);
    return (((!x.dead) && (x.repl < i64(0))) && index(g.blocks, x.block).alive);
}
i64 count_ir(IR g) {
    i64 n = i64(0);
    i64 i = i64(0);
    while ((i < len(g.ins))) {
        if ((((live_inst(g, i) && (index(g.ins, i).op != str("arg"))) &&
              (index(g.ins, i).op != str("const"))) &&
             (index(g.ins, i).op != str("string")))) {
            n = wrap_add(n, i64(1));
        }
        i = wrap_add(i, i64(1));
    }
    return n;
}
str instruction_key(IR g, i64 id) {
    Inst x = index(g.ins, id);
    Arr<str> parts = {x.op, x.ty, x.text};
    Arr<i64> xs = operands(x);
    i64 i = i64(0);
    while ((i < len(xs))) {
        i64 v = canonical(g, index(xs, i));
        Inst operand = index(g.ins, v);
        if ((operand.op == str("const"))) {
            push(parts, wrap_add(wrap_add(wrap_add(str("constant/"), operand.ty), str("/")),
                                 operand.text));
        } else {
            if ((operand.op == str("string"))) {
                push(parts, wrap_add(str("string/"), escaped_bytes(operand.text)));
            } else {
                push(parts, wrap_add(str("value/"), itoa(v)));
            }
        }
        i = wrap_add(i, i64(1));
    }
    return join(parts, str(":"));
}
str dump_ir(IR g) {
    Arr<str> lines = {
        wrap_add(wrap_add(wrap_add(wrap_add(str("function "), index(g.s.functions, g.fid).module),
                                   str("::")),
                          index(g.s.functions, g.fid).name),
                 str("\n"))};
    i64 b = i64(0);
    while ((b < len(g.blocks))) {
        if (index(g.blocks, b).alive) {
            push(lines, wrap_add(wrap_add(str("b"), itoa(b)), str(":\n")));
            i64 i = i64(0);
            while ((i < len(index(g.blocks, b).code))) {
                i64 id = index(index(g.blocks, b).code, i);
                Inst x = index(g.ins, id);
                if ((live_inst(g, id) && (x.block == b))) {
                    Arr<str> args = {};
                    Arr<i64> xs = operands(x);
                    i64 j = i64(0);
                    while ((j < len(xs))) {
                        str part = wrap_add(str("%"), itoa(canonical(g, index(xs, j))));
                        if ((x.op == str("phi"))) {
                            part = wrap_add(wrap_add(part, str("@b")), itoa(index(x.labels, j)));
                        }
                        push(args, part);
                        j = wrap_add(j, i64(1));
                    }
                    str text = x.text;
                    if ((x.op == str("string"))) {
                        text = escaped_bytes(text);
                    }
                    push(
                        lines,
                        wrap_add(
                            wrap_add(
                                wrap_add(
                                    wrap_add(
                                        wrap_add(
                                            wrap_add(wrap_add(wrap_add(wrap_add(wrap_add(str("  %"),
                                                                                         itoa(id)),
                                                                                str(":")),
                                                                       x.ty),
                                                              str(" = ")),
                                                     x.op),
                                            str(" ")),
                                        text),
                                    str(" ")),
                                join(args, str(", "))),
                            str("\n")));
                }
                i = wrap_add(i, i64(1));
            }
            Arr<str> args = {};
            i = i64(0);
            while ((i < len(index(g.blocks, b).values))) {
                push(args,
                     wrap_add(str("%"), itoa(canonical(g, index(index(g.blocks, b).values, i)))));
                i = wrap_add(i, i64(1));
            }
            i = i64(0);
            while ((i < len(index(g.blocks, b).targets))) {
                push(args, wrap_add(str("b"), itoa(index(index(g.blocks, b).targets, i))));
                i = wrap_add(i, i64(1));
            }
            push(lines,
                 wrap_add(wrap_add(wrap_add(wrap_add(str("  "), index(g.blocks, b).term), str(" ")),
                                   join(args, str(", "))),
                          str("\n")));
        }
        b = wrap_add(b, i64(1));
    }
    return join(lines, str(""));
}
bool foldable(IR g, Inst x) {
    if ((((((((!(((pure(x.op) || (x.op == str("div"))) || (x.op == str("rem"))))) ||
              (x.op == str("record"))) ||
             (x.op == str("extract"))) ||
            (x.op == str("insert"))) ||
           (x.op == str("arg"))) ||
          (x.op == str("string"))) ||
         (x.op == str("const")))) {
        return false;
    }
    Arr<i64> xs = operands(x);
    i64 i = i64(0);
    while ((i < len(xs))) {
        if ((index(g.ins, canonical(g, index(xs, i))).op != str("const"))) {
            return false;
        }
        i = wrap_add(i, i64(1));
    }
    if (((((x.op == str("div")) || (x.op == str("rem")))) &&
         (parse_int(index(g.ins, canonical(g, x.b)).text) == i64(0)))) {
        return false;
    }
    return true;
}
i64 eval_scalar(str op, i64 a, i64 b, str ty, str source) {
    i64 n = i64(0);
    if ((op == str("add"))) {
        n = wrap_add(a, b);
    } else {
        if ((op == str("sub"))) {
            n = wrap_sub(a, b);
        } else {
            if ((op == str("mul"))) {
                n = wrap_mul(a, b);
            } else {
                if ((op == str("div"))) {
                    n = wrap_div(a, b);
                } else {
                    if ((op == str("rem"))) {
                        n = wrap_rem(a, b);
                    } else {
                        if ((op == str("shl"))) {
                            i64 mask = i64(63);
                            if ((ty == str("u8"))) {
                                mask = i64(7);
                            }
                            n = wrap_shl(a, ((b & mask)));
                        } else {
                            if ((op == str("shr"))) {
                                i64 mask = i64(63);
                                if ((ty == str("u8"))) {
                                    mask = i64(7);
                                }
                                n = (a >> ((b & mask)));
                            } else {
                                if ((op == str("and"))) {
                                    n = (a & b);
                                } else {
                                    if ((op == str("or"))) {
                                        n = (a | b);
                                    } else {
                                        if ((op == str("xor"))) {
                                            n = (a ^ b);
                                        } else {
                                            if ((op == str("neg"))) {
                                                n = wrap_sub(i64(0), a);
                                            } else {
                                                if ((op == str("inv"))) {
                                                    n = (~a);
                                                } else {
                                                    if ((op == str("not"))) {
                                                        if ((a == i64(0))) {
                                                            n = i64(1);
                                                        }
                                                    } else {
                                                        if ((op == str("eq"))) {
                                                            if ((a == b)) {
                                                                n = i64(1);
                                                            }
                                                        } else {
                                                            if ((op == str("ne"))) {
                                                                if ((a != b)) {
                                                                    n = i64(1);
                                                                }
                                                            } else {
                                                                if ((op == str("lt"))) {
                                                                    if ((a < b)) {
                                                                        n = i64(1);
                                                                    }
                                                                } else {
                                                                    if ((op == str("le"))) {
                                                                        if ((a <= b)) {
                                                                            n = i64(1);
                                                                        }
                                                                    } else {
                                                                        if ((op == str("gt"))) {
                                                                            if ((a > b)) {
                                                                                n = i64(1);
                                                                            }
                                                                        } else {
                                                                            if ((op == str("ge"))) {
                                                                                if ((a >= b)) {
                                                                                    n = i64(1);
                                                                                }
                                                                            } else {
                                                                                if ((op ==
                                                                                     str("cast"))) {
                                                                                    n = a;
                                                                                    if ((ty ==
                                                                                         str("boo"
                                                                                             "l"))) {
                                                                                        n = i64(0);
                                                                                        if ((a !=
                                                                                             i64(0))) {
                                                                                            n = i64(
                                                                                                1);
                                                                                        }
                                                                                    }
                                                                                }
                                                                            }
                                                                        }
                                                                    }
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if ((ty == str("u8"))) {
        n = (n & i64(255));
    }
    return n;
}
void turn_constant(IR g, i64 id, i64 n) {
    index(g.ins, id).op = str("const");
    index(g.ins, id).a = wrap_sub(0, i64(1));
    index(g.ins, id).b = wrap_sub(0, i64(1));
    index(g.ins, id).args = {};
    index(g.ins, id).labels = {};
    index(g.ins, id).text = itoa(n);
}
void simplify(IR g) {
    i64 i = i64(0);
    while ((i < len(g.ins))) {
        if (live_inst(g, i)) {
            Inst x = index(g.ins, i);
            if ((x.op == str("phi"))) {
                trivial_phi(g, i);
            } else {
                if (foldable(g, x)) {
                    i64 a = i64(0);
                    i64 b = i64(0);
                    if ((x.a >= i64(0))) {
                        a = parse_int(index(g.ins, canonical(g, x.a)).text);
                    }
                    if ((x.b >= i64(0))) {
                        b = parse_int(index(g.ins, canonical(g, x.b)).text);
                    }
                    turn_constant(g, i, eval_scalar(x.op, a, b, x.ty, str("")));
                } else {
                    if ((x.op == str("extract"))) {
                        i64 a = canonical(g, x.a);
                        i64 f = parse_int(x.text);
                        if ((index(g.ins, a).op == str("record"))) {
                            index(g.ins, i).repl = index(index(g.ins, a).args, f);
                        } else {
                            if (((index(g.ins, a).op == str("insert")) &&
                                 (index(g.ins, a).text == x.text))) {
                                index(g.ins, i).repl = index(g.ins, a).b;
                            }
                        }
                    } else {
                        if ((x.b >= i64(0))) {
                            i64 a = canonical(g, x.a);
                            i64 b = canonical(g, x.b);
                            Inst rhs = index(g.ins, b);
                            if ((rhs.op == str("const"))) {
                                i64 n = parse_int(rhs.text);
                                if ((((((((((x.op == str("add")) || (x.op == str("sub"))) ||
                                           (x.op == str("or"))) ||
                                          (x.op == str("xor"))) ||
                                         (x.op == str("shl"))) ||
                                        (x.op == str("shr")))) &&
                                      (n == i64(0))) ||
                                     ((x.op == str("mul")) && (n == i64(1))))) {
                                    index(g.ins, i).repl = a;
                                }
                                if (((((x.op == str("mul")) || (x.op == str("and")))) &&
                                     (n == i64(0)))) {
                                    turn_constant(g, i, i64(0));
                                }
                            }
                            if (((a == b) && (((x.op == str("sub")) || (x.op == str("xor")))))) {
                                turn_constant(g, i, i64(0));
                            }
                        }
                    }
                }
            }
        }
        i = wrap_add(i, i64(1));
    }
}
void rebuild_cfg(IR g) {
    i64 b = i64(0);
    while ((b < len(g.blocks))) {
        index(g.blocks, b).preds = {};
        b = wrap_add(b, i64(1));
    }
    b = i64(0);
    while ((b < len(g.blocks))) {
        if (index(g.blocks, b).alive) {
            i64 j = i64(0);
            while ((j < len(index(g.blocks, b).targets))) {
                i64 t = index(index(g.blocks, b).targets, j);
                if ((!has(index(g.blocks, t).preds, b))) {
                    push(index(g.blocks, t).preds, b);
                }
                j = wrap_add(j, i64(1));
            }
        }
        b = wrap_add(b, i64(1));
    }
    i64 i = i64(0);
    while ((i < len(g.ins))) {
        if ((live_inst(g, i) && (index(g.ins, i).op == str("phi")))) {
            Arr<i64> args = {};
            Arr<i64> labels = {};
            i64 j = i64(0);
            while ((j < len(index(g.ins, i).args))) {
                i64 p = index(index(g.ins, i).labels, j);
                if ((has(index(g.blocks, index(g.ins, i).block).preds, p) && (!has(labels, p)))) {
                    push(args, index(index(g.ins, i).args, j));
                    push(labels, p);
                }
                j = wrap_add(j, i64(1));
            }
            index(g.ins, i).args = args;
            index(g.ins, i).labels = labels;
            trivial_phi(g, i);
        }
        i = wrap_add(i, i64(1));
    }
}
void cfg_simplify(IR g) {
    Arr<i64> reachable = {};
    i64 b = i64(0);
    while ((b < len(g.blocks))) {
        push(reachable, i64(0));
        b = wrap_add(b, i64(1));
    }
    Arr<i64> work = {i64(0)};
    while ((len(work) > i64(0))) {
        i64 from = index(work, wrap_sub(len(work), i64(1)));
        pop(work);
        if ((index(reachable, from) == i64(0))) {
            index(reachable, from) = i64(1);
            i64 i = i64(0);
            while ((i < len(index(g.blocks, from).targets))) {
                push(work, index(index(g.blocks, from).targets, i));
                i = wrap_add(i, i64(1));
            }
        }
    }
    b = i64(0);
    while ((b < len(g.blocks))) {
        index(g.blocks, b).alive = (index(reachable, b) == i64(1));
        b = wrap_add(b, i64(1));
    }
    rebuild_cfg(g);
    bool changed = true;
    while (changed) {
        changed = false;
        b = i64(0);
        while ((b < len(g.blocks))) {
            if ((index(g.blocks, b).alive && (index(g.blocks, b).term == str("jump")))) {
                i64 t = index(index(g.blocks, b).targets, i64(0));
                if ((((t != i64(0)) && (t != b)) && (len(index(g.blocks, t).preds) == i64(1)))) {
                    i64 j = i64(0);
                    while ((j < len(index(g.blocks, t).code))) {
                        i64 id = index(index(g.blocks, t).code, j);
                        index(g.ins, id).block = b;
                        push(index(g.blocks, b).code, id);
                        j = wrap_add(j, i64(1));
                    }
                    index(g.blocks, b).term = index(g.blocks, t).term;
                    index(g.blocks, b).values = index(g.blocks, t).values;
                    index(g.blocks, b).targets = index(g.blocks, t).targets;
                    index(g.blocks, t).alive = false;
                    j = i64(0);
                    while ((j < len(g.ins))) {
                        if ((index(g.ins, j).op == str("phi"))) {
                            i64 k = i64(0);
                            while ((k < len(index(g.ins, j).labels))) {
                                if ((index(index(g.ins, j).labels, k) == t)) {
                                    index(index(g.ins, j).labels, k) = b;
                                }
                                k = wrap_add(k, i64(1));
                            }
                        }
                        j = wrap_add(j, i64(1));
                    }
                    changed = true;
                    rebuild_cfg(g);
                }
            }
            b = wrap_add(b, i64(1));
        }
    }
}
void dce(IR g) {
    Arr<i64> marked = {};
    Arr<i64> work = {};
    i64 i = i64(0);
    while ((i < len(g.ins))) {
        push(marked, i64(0));
        Inst x = index(g.ins, i);
        if ((((live_inst(g, i) && (!pure(x.op))) && (x.op != str("phi"))) &&
             (x.op != str("len")))) {
            push(work, i);
        }
        i = wrap_add(i, i64(1));
    }
    i64 b = i64(0);
    while ((b < len(g.blocks))) {
        if (index(g.blocks, b).alive) {
            i = i64(0);
            while ((i < len(index(g.blocks, b).values))) {
                push(work, index(index(g.blocks, b).values, i));
                i = wrap_add(i, i64(1));
            }
        }
        b = wrap_add(b, i64(1));
    }
    while ((len(work) > i64(0))) {
        i64 id = canonical(g, index(work, wrap_sub(len(work), i64(1))));
        pop(work);
        if ((index(marked, id) == i64(0))) {
            index(marked, id) = i64(1);
            Arr<i64> xs = operands(index(g.ins, id));
            i = i64(0);
            while ((i < len(xs))) {
                push(work, index(xs, i));
                i = wrap_add(i, i64(1));
            }
        }
    }
    i = i64(0);
    while ((i < len(g.ins))) {
        if ((index(marked, i) == i64(0))) {
            index(g.ins, i).dead = true;
        }
        i = wrap_add(i, i64(1));
    }
}
void sccp(IR g) {
    i64 nb = len(g.blocks);
    Arr<i64> lattice = {};
    Arr<i64> vals = {};
    Arr<Arr<i64>> users = {};
    i64 i = i64(0);
    while ((i < len(g.ins))) {
        push(lattice, i64(0));
        push(vals, i64(0));
        Arr<i64> empty = {};
        push(users, empty);
        i = wrap_add(i, i64(1));
    }
    i64 b = i64(0);
    while ((b < nb)) {
        i = i64(0);
        while ((i < len(index(g.blocks, b).code))) {
            i64 id = index(index(g.blocks, b).code, i);
            if (live_inst(g, id)) {
                Arr<i64> xs = operands(index(g.ins, id));
                i64 j = i64(0);
                while ((j < len(xs))) {
                    push(index(users, canonical(g, index(xs, j))), b);
                    j = wrap_add(j, i64(1));
                }
            }
            i = wrap_add(i, i64(1));
        }
        i = i64(0);
        while ((i < len(index(g.blocks, b).values))) {
            push(index(users, canonical(g, index(index(g.blocks, b).values, i))), b);
            i = wrap_add(i, i64(1));
        }
        b = wrap_add(b, i64(1));
    }
    Arr<i64> edges = {};
    Arr<i64> executable = {};
    Arr<i64> queued = {};
    i = i64(0);
    while ((i < wrap_mul(nb, nb))) {
        push(edges, i64(0));
        i = wrap_add(i, i64(1));
    }
    i = i64(0);
    while ((i < nb)) {
        push(executable, i64(0));
        push(queued, i64(0));
        i = wrap_add(i, i64(1));
    }
    Arr<i64> work = {i64(0)};
    index(queued, i64(0)) = i64(1);
    index(executable, i64(0)) = i64(1);
    i64 cursor = i64(0);
    while ((cursor < len(work))) {
        b = index(work, cursor);
        cursor = wrap_add(cursor, i64(1));
        index(queued, b) = i64(0);
        i = i64(0);
        while ((i < len(index(g.blocks, b).code))) {
            i64 id = index(index(g.blocks, b).code, i);
            if (live_inst(g, id)) {
                Inst x = index(g.ins, id);
                i64 state = i64(0);
                i64 value = i64(0);
                if ((x.op == str("const"))) {
                    state = i64(1);
                    value = parse_int(x.text);
                } else {
                    if ((x.op == str("phi"))) {
                        i64 j = i64(0);
                        while ((j < len(x.args))) {
                            if ((index(edges, wrap_add(wrap_mul(index(x.labels, j), nb), b)) ==
                                 i64(1))) {
                                i64 a = canonical(g, index(x.args, j));
                                if ((index(lattice, a) == i64(2))) {
                                    state = i64(2);
                                } else {
                                    if ((index(lattice, a) == i64(1))) {
                                        if ((state == i64(0))) {
                                            state = i64(1);
                                            value = index(vals, a);
                                        } else {
                                            if (((state == i64(1)) && (value != index(vals, a)))) {
                                                state = i64(2);
                                            }
                                        }
                                    }
                                }
                            }
                            j = wrap_add(j, i64(1));
                        }
                    } else {
                        if (((((((((pure(x.op) || (x.op == str("div"))) || (x.op == str("rem")))) &&
                                 (x.op != str("arg"))) &&
                                (x.op != str("string"))) &&
                               (x.op != str("record"))) &&
                              (x.op != str("extract"))) &&
                             (x.op != str("insert")))) {
                            state = i64(1);
                            Arr<i64> xs = operands(x);
                            i64 j = i64(0);
                            bool unknown = false;
                            while ((j < len(xs))) {
                                i64 a = canonical(g, index(xs, j));
                                if ((index(lattice, a) == i64(2))) {
                                    state = i64(2);
                                }
                                if ((index(lattice, a) == i64(0))) {
                                    unknown = true;
                                }
                                j = wrap_add(j, i64(1));
                            }
                            if (((state == i64(1)) && unknown)) {
                                state = i64(0);
                            }
                            if ((state == i64(1))) {
                                i64 a = i64(0);
                                i64 bv = i64(0);
                                if ((x.a >= i64(0))) {
                                    a = index(vals, canonical(g, x.a));
                                }
                                if ((x.b >= i64(0))) {
                                    bv = index(vals, canonical(g, x.b));
                                }
                                if (((((x.op == str("div")) || (x.op == str("rem")))) &&
                                     (bv == i64(0)))) {
                                    state = i64(2);
                                } else {
                                    value = eval_scalar(x.op, a, bv, x.ty, str(""));
                                }
                            }
                        } else {
                            state = i64(2);
                        }
                    }
                }
                bool changed = false;
                if ((state > index(lattice, id))) {
                    index(lattice, id) = state;
                    index(vals, id) = value;
                    changed = true;
                } else {
                    if ((((state == i64(1)) && (index(lattice, id) == i64(1))) &&
                         (index(vals, id) != value))) {
                        index(lattice, id) = i64(2);
                        changed = true;
                    }
                }
                if (changed) {
                    i64 j = i64(0);
                    while ((j < len(index(users, id)))) {
                        i64 target = index(index(users, id), j);
                        if (((index(executable, target) == i64(1)) &&
                             (index(queued, target) == i64(0)))) {
                            push(work, target);
                            index(queued, target) = i64(1);
                        }
                        j = wrap_add(j, i64(1));
                    }
                }
            }
            i = wrap_add(i, i64(1));
        }
        Arr<i64> targets = {};
        Block block = index(g.blocks, b);
        if ((block.term == str("jump"))) {
            targets = block.targets;
        } else {
            if ((block.term == str("branch"))) {
                i64 cond = canonical(g, index(block.values, i64(0)));
                if ((index(lattice, cond) == i64(1))) {
                    i64 branch = i64(1);
                    if ((index(vals, cond) != i64(0))) {
                        branch = i64(0);
                    }
                    push(targets, index(block.targets, branch));
                } else {
                    if ((index(lattice, cond) == i64(2))) {
                        targets = block.targets;
                    }
                }
            }
        }
        i = i64(0);
        while ((i < len(targets))) {
            i64 target = index(targets, i);
            if ((index(edges, wrap_add(wrap_mul(b, nb), target)) == i64(0))) {
                index(edges, wrap_add(wrap_mul(b, nb), target)) = i64(1);
                index(executable, target) = i64(1);
                if ((index(queued, target) == i64(0))) {
                    push(work, target);
                    index(queued, target) = i64(1);
                }
            }
            i = wrap_add(i, i64(1));
        }
    }
    i = i64(0);
    while ((i < len(g.ins))) {
        if (((live_inst(g, i) && (index(lattice, i) == i64(1))) &&
             (index(g.ins, i).op != str("const")))) {
            turn_constant(g, i, index(vals, i));
        }
        i = wrap_add(i, i64(1));
    }
    b = i64(0);
    while ((b < nb)) {
        index(g.blocks, b).alive = (index(executable, b) == i64(1));
        if ((index(g.blocks, b).alive && (index(g.blocks, b).term == str("branch")))) {
            i64 cond = canonical(g, index(index(g.blocks, b).values, i64(0)));
            if ((index(lattice, cond) == i64(1))) {
                i64 branch = i64(1);
                if ((index(vals, cond) != i64(0))) {
                    branch = i64(0);
                }
                index(g.blocks, b).targets = {index(index(g.blocks, b).targets, branch)};
                index(g.blocks, b).values = {};
                index(g.blocks, b).term = str("jump");
            }
        }
        b = wrap_add(b, i64(1));
    }
    rebuild_cfg(g);
}
Arr<Arr<i64>> dominators(IR g) {
    Arr<Arr<i64>> dom = {};
    i64 n = len(g.blocks);
    i64 b = i64(0);
    while ((b < n)) {
        Arr<i64> row = {};
        i64 j = i64(0);
        while ((j < n)) {
            i64 v = i64(0);
            if (((b == j) ||
                 (((b != i64(0)) && index(g.blocks, b).alive) && index(g.blocks, j).alive))) {
                v = i64(1);
            }
            push(row, v);
            j = wrap_add(j, i64(1));
        }
        push(dom, row);
        b = wrap_add(b, i64(1));
    }
    bool change = true;
    while (change) {
        change = false;
        b = i64(1);
        while ((b < n)) {
            if (index(g.blocks, b).alive) {
                i64 j = i64(0);
                while ((j < n)) {
                    i64 v = i64(1);
                    i64 p = i64(0);
                    while ((p < len(index(g.blocks, b).preds))) {
                        if ((index(index(dom, index(index(g.blocks, b).preds, p)), j) == i64(0))) {
                            v = i64(0);
                        }
                        p = wrap_add(p, i64(1));
                    }
                    if ((j == b)) {
                        v = i64(1);
                    }
                    if ((index(index(dom, b), j) != v)) {
                        index(index(dom, b), j) = v;
                        change = true;
                    }
                    j = wrap_add(j, i64(1));
                }
            }
            b = wrap_add(b, i64(1));
        }
    }
    return dom;
}
i64 position(IR g, i64 id) {
    Block block = index(g.blocks, index(g.ins, id).block);
    i64 i = i64(0);
    while ((i < len(block.code))) {
        if ((index(block.code, i) == id)) {
            return i;
        }
        i = wrap_add(i, i64(1));
    }
    return wrap_sub(0, i64(1));
}
Arr<i64> dominator_order(IR g, Arr<Arr<i64>> dom) {
    Arr<Arr<i64>> children = {};
    i64 b = i64(0);
    while ((b < len(g.blocks))) {
        Arr<i64> empty = {};
        push(children, empty);
        b = wrap_add(b, i64(1));
    }
    b = i64(1);
    while ((b < len(g.blocks))) {
        if (index(g.blocks, b).alive) {
            i64 parent = i64(0);
            i64 best = wrap_sub(0, i64(1));
            i64 d = i64(0);
            while ((d < len(g.blocks))) {
                if (((d != b) && (index(index(dom, b), d) == i64(1)))) {
                    i64 depth = i64(0);
                    i64 j = i64(0);
                    while ((j < len(g.blocks))) {
                        depth = wrap_add(depth, index(index(dom, d), j));
                        j = wrap_add(j, i64(1));
                    }
                    if ((depth > best)) {
                        best = depth;
                        parent = d;
                    }
                }
                d = wrap_add(d, i64(1));
            }
            push(index(children, parent), b);
        }
        b = wrap_add(b, i64(1));
    }
    Arr<i64> result = {};
    Arr<i64> work = {i64(0)};
    while ((len(work) > i64(0))) {
        i64 n = index(work, wrap_sub(len(work), i64(1)));
        pop(work);
        push(result, n);
        i64 j = wrap_sub(len(index(children, n)), i64(1));
        while ((j >= i64(0))) {
            push(work, index(index(children, n), j));
            j = wrap_sub(j, i64(1));
        }
    }
    return result;
}
void gvn(IR g) {
    Arr<Arr<i64>> dom = dominators(g);
    StringMap table = map_new();
    Arr<i64> ids = {};
    Arr<i64> next = {};
    Arr<i64> order = dominator_order(g, dom);
    i64 bi = i64(0);
    while ((bi < len(order))) {
        i64 block = index(order, bi);
        i64 ci = i64(0);
        while ((ci < len(index(g.blocks, block).code))) {
            i64 i = index(index(g.blocks, block).code, ci);
            Inst x = index(g.ins, i);
            if ((((((live_inst(g, i) && (x.block == block)) && pure(x.op)) &&
                   (x.op != str("const"))) &&
                  (x.op != str("arg"))) &&
                 (x.op != str("string")))) {
                str key = instruction_key(g, i);
                i64 head = map_get(table, key);
                i64 j = head;
                bool found = false;
                while (((j >= i64(0)) && (!found))) {
                    i64 other = canonical(g, index(ids, j));
                    i64 ob = index(g.ins, other).block;
                    if (((index(index(dom, x.block), ob) == i64(1)) &&
                         (((ob != x.block) || (position(g, other) < position(g, i)))))) {
                        index(g.ins, i).repl = other;
                        found = true;
                    }
                    j = index(next, j);
                }
                if ((!found)) {
                    map_put(table, key, len(ids));
                    push(ids, i);
                    push(next, head);
                }
            }
            ci = wrap_add(ci, i64(1));
        }
        bi = wrap_add(bi, i64(1));
    }
}
void licm(IR g) {
    Arr<Arr<i64>> dom = dominators(g);
    i64 n = len(g.blocks);
    i64 h = i64(0);
    while ((h < n)) {
        if (index(g.blocks, h).alive) {
            Arr<i64> members = {};
            Arr<i64> work = {};
            i64 i = i64(0);
            while ((i < n)) {
                push(members, i64(0));
                i = wrap_add(i, i64(1));
            }
            index(members, h) = i64(1);
            i64 latch = i64(0);
            while ((latch < n)) {
                if (((index(g.blocks, latch).alive && has(index(g.blocks, latch).targets, h)) &&
                     (index(index(dom, latch), h) == i64(1)))) {
                    push(work, latch);
                }
                latch = wrap_add(latch, i64(1));
            }
            if ((len(work) > i64(0))) {
                while ((len(work) > i64(0))) {
                    i64 b = index(work, wrap_sub(len(work), i64(1)));
                    pop(work);
                    if ((index(members, b) == i64(0))) {
                        index(members, b) = i64(1);
                        i = i64(0);
                        while ((i < len(index(g.blocks, b).preds))) {
                            push(work, index(index(g.blocks, b).preds, i));
                            i = wrap_add(i, i64(1));
                        }
                    }
                }
                i64 pre = wrap_sub(0, i64(1));
                i64 count = i64(0);
                i = i64(0);
                while ((i < len(index(g.blocks, h).preds))) {
                    i64 p = index(index(g.blocks, h).preds, i);
                    if ((index(members, p) == i64(0))) {
                        pre = p;
                        count = wrap_add(count, i64(1));
                    }
                    i = wrap_add(i, i64(1));
                }
                if (((count == i64(1)) && (index(g.blocks, pre).term == str("jump")))) {
                    bool changed = true;
                    while (changed) {
                        changed = false;
                        i = i64(0);
                        while ((i < len(g.ins))) {
                            Inst x = index(g.ins, i);
                            if ((((((live_inst(g, i) && (index(members, x.block) == i64(1))) &&
                                    pure(x.op)) &&
                                   (x.op != str("const"))) &&
                                  (x.op != str("arg"))) &&
                                 (x.op != str("string")))) {
                                Arr<i64> xs = operands(x);
                                bool safe = true;
                                i64 j = i64(0);
                                while ((j < len(xs))) {
                                    i64 v = canonical(g, index(xs, j));
                                    if (((((index(members, index(g.ins, v).block) == i64(1)) &&
                                           (index(g.ins, v).op != str("const"))) &&
                                          (index(g.ins, v).op != str("arg"))) &&
                                         (index(g.ins, v).op != str("string")))) {
                                        safe = false;
                                    }
                                    j = wrap_add(j, i64(1));
                                }
                                if (safe) {
                                    index(g.ins, i).block = pre;
                                    push(index(g.blocks, pre).code, i);
                                    changed = true;
                                }
                            }
                            i = wrap_add(i, i64(1));
                        }
                    }
                }
            }
        }
        h = wrap_add(h, i64(1));
    }
}
void verify_ir(IR g) {
    Arr<Arr<i64>> dom = dominators(g);
    i64 b = i64(0);
    while ((b < len(g.blocks))) {
        if (index(g.blocks, b).alive) {
            if ((index(g.blocks, b).term == str(""))) {
                error(g.s, index(g.s.functions, g.fid).tok,
                      str("IR verification: unterminated block"));
            }
            Block term = index(g.blocks, b);
            i64 where = index(g.s.functions, g.fid).tok;
            if ((!term.sealed)) {
                error(g.s, where, str("IR verification: unsealed live block"));
            }
            if ((term.term == str("branch"))) {
                if (((len(term.targets) != i64(2)) || (len(term.values) != i64(1)))) {
                    error(g.s, where, str("IR verification: invalid branch"));
                }
                expect(g, index(g.ins, canonical(g, index(term.values, i64(0)))).ty, str("bool"),
                       where);
            }
            if (((term.term == str("jump")) && (len(term.targets) != i64(1)))) {
                error(g.s, where, str("IR verification: invalid jump"));
            }
            if ((term.term == str("return"))) {
                str ret = index(g.s.functions, g.fid).ret;
                if ((ret == str("void"))) {
                    if ((len(term.values) != i64(0))) {
                        error(g.s, where, str("IR verification: invalid void return"));
                    }
                } else {
                    if ((len(term.values) != i64(1))) {
                        error(g.s, where, str("IR verification: missing return value"));
                    }
                    expect(g, index(g.ins, canonical(g, index(term.values, i64(0)))).ty, ret,
                           where);
                }
            }
            i64 vi = i64(0);
            while ((vi < len(term.values))) {
                i64 v = canonical(g, index(term.values, vi));
                Inst def = index(g.ins, v);
                if ((def.dead || ((((def.op != str("const")) && (def.op != str("string"))) &&
                                   (def.op != str("arg"))) &&
                                  (index(index(dom, b), def.block) == i64(0))))) {
                    error(g.s, where, str("IR verification: invalid terminator operand"));
                }
                vi = wrap_add(vi, i64(1));
            }
            i64 i = i64(0);
            while ((i < len(index(g.blocks, b).code))) {
                i64 id = index(index(g.blocks, b).code, i);
                Inst x = index(g.ins, id);
                if ((live_inst(g, id) && (x.block == b))) {
                    if ((find({str("add"), str("sub"), str("mul"), str("div"), str("rem"),
                               str("shl"), str("shr"), str("and"), str("or"), str("xor")},
                              x.op) >= i64(0))) {
                        expect(g, index(g.ins, canonical(g, x.a)).ty, x.ty, where);
                        expect(g, index(g.ins, canonical(g, x.b)).ty, x.ty, where);
                    }
                    if ((find({str("eq"), str("ne"), str("lt"), str("le"), str("gt"), str("ge")},
                              x.op) >= i64(0))) {
                        expect(g, x.ty, str("bool"), where);
                        expect(g, index(g.ins, canonical(g, x.a)).ty,
                               index(g.ins, canonical(g, x.b)).ty, where);
                    }
                    if ((x.op == str("phi"))) {
                        if (((len(x.args) != len(index(g.blocks, b).preds)) ||
                             (len(x.labels) != len(x.args)))) {
                            error(g.s, index(g.s.functions, g.fid).tok,
                                  str("IR verification: phi predecessor mismatch"));
                        }
                        i64 pi = i64(0);
                        while ((pi < len(x.labels))) {
                            if ((!has(index(g.blocks, b).preds, index(x.labels, pi)))) {
                                error(g.s, where,
                                      str("IR verification: phi references a non-predecessor"));
                            }
                            i64 pj = i64(0);
                            while ((pj < pi)) {
                                if ((index(x.labels, pj) == index(x.labels, pi))) {
                                    error(g.s, where, str("IR verification: duplicate phi edge"));
                                }
                                pj = wrap_add(pj, i64(1));
                            }
                            pi = wrap_add(pi, i64(1));
                        }
                    }
                    Arr<i64> xs = operands(x);
                    i64 j = i64(0);
                    while ((j < len(xs))) {
                        i64 v = canonical(g, index(xs, j));
                        Inst def = index(g.ins, v);
                        i64 useblock = b;
                        if ((x.op == str("phi"))) {
                            useblock = index(x.labels, j);
                            expect(g, def.ty, x.ty, index(g.s.functions, g.fid).tok);
                        }
                        if (def.dead) {
                            error(g.s, index(g.s.functions, g.fid).tok,
                                  str("IR verification: use of deleted value"));
                        }
                        if ((((def.op != str("const")) && (def.op != str("arg"))) &&
                             (def.op != str("string")))) {
                            if ((index(index(dom, useblock), def.block) == i64(0))) {
                                error(
                                    g.s, index(g.s.functions, g.fid).tok,
                                    wrap_add(
                                        wrap_add(
                                            wrap_add(
                                                str("IR verification: non-dominating definition %"),
                                                itoa(v)),
                                            str(" at %")),
                                        itoa(id)));
                            }
                            if (((((x.op != str("phi")) && (def.block == b)) &&
                                  (def.op != str("phi"))) &&
                                 (position(g, v) >= position(g, id)))) {
                                error(g.s, index(g.s.functions, g.fid).tok,
                                      str("IR verification: definition after use"));
                            }
                        }
                        j = wrap_add(j, i64(1));
                    }
                }
                i = wrap_add(i, i64(1));
            }
        }
        b = wrap_add(b, i64(1));
    }
}
IR new_ir(State s, i64 fid) {
    return IR{s, fid, {}, {}, {}, {}, {i64(0)}, {i64(0)}, {}, {}};
}
i64 new_block(IR g) {
    i64 id = len(g.blocks);
    push(g.blocks, Block{{}, {}, {}, {}, false, str(""), {}, {}, true});
    return id;
}
i64 emit(IR g, str op, str ty, i64 a, i64 b, Arr<i64> xs, str text) {
    i64 id = len(g.ins);
    i64 block = index(g.current, i64(0));
    push(g.ins, Inst{op, ty, a, b, xs, {}, text, block, wrap_sub(0, i64(1)), false});
    push(index(g.blocks, block).code, id);
    return id;
}
i64 canonical(IR g, i64 id) {
    i64 n = id;
    while (((n >= i64(0)) && (index(g.ins, n).repl >= i64(0)))) {
        n = index(g.ins, n).repl;
    }
    return n;
}
i64 constant(IR g, str ty, i64 n) {
    return emit(g, str("const"), ty, wrap_sub(0, i64(1)), wrap_sub(0, i64(1)), {}, itoa(n));
}
void write_var(IR g, i64 block, i64 varid, i64 value) {
    while ((len(index(g.blocks, block).defs) <= varid)) {
        push(index(g.blocks, block).defs, wrap_sub(0, i64(1)));
    }
    index(index(g.blocks, block).defs, varid) = value;
}
i64 trivial_phi(IR g, i64 id) {
    i64 same = wrap_sub(0, i64(1));
    i64 i = i64(0);
    while ((i < len(index(g.ins, id).args))) {
        i64 v = canonical(g, index(index(g.ins, id).args, i));
        if ((v != id)) {
            if (((same >= i64(0)) && (same != v))) {
                return id;
            }
            same = v;
        }
        i = wrap_add(i, i64(1));
    }
    if ((same >= i64(0))) {
        index(g.ins, id).repl = same;
    }
    return canonical(g, id);
}
i64 make_phi(IR g, i64 block, i64 varid) {
    i64 old = index(g.current, i64(0));
    index(g.current, i64(0)) = block;
    i64 id = emit(g, str("phi"), index(g.vars, varid).ty, wrap_sub(0, i64(1)), wrap_sub(0, i64(1)),
                  {}, itoa(varid));
    index(g.current, i64(0)) = old;
    write_var(g, block, varid, id);
    return id;
}
i64 read_var(IR g, i64 block, i64 varid) {
    if (((varid < len(index(g.blocks, block).defs)) &&
         (index(index(g.blocks, block).defs, varid) >= i64(0)))) {
        return canonical(g, index(index(g.blocks, block).defs, varid));
    }
    i64 value = wrap_sub(0, i64(1));
    if ((!index(g.blocks, block).sealed)) {
        value = make_phi(g, block, varid);
        push(index(g.blocks, block).pending, value);
    } else {
        if ((len(index(g.blocks, block).preds) == i64(1))) {
            value = read_var(g, index(index(g.blocks, block).preds, i64(0)), varid);
        } else {
            if ((len(index(g.blocks, block).preds) == i64(0))) {
                error(g.s, index(g.s.functions, g.fid).tok,
                      wrap_add(str("variable has no reaching definition: "),
                               index(g.vars, varid).name));
            }
            value = make_phi(g, block, varid);
            i64 i = i64(0);
            while ((i < len(index(g.blocks, block).preds))) {
                i64 p = index(index(g.blocks, block).preds, i);
                push(index(g.ins, value).args, read_var(g, p, varid));
                push(index(g.ins, value).labels, p);
                i = wrap_add(i, i64(1));
            }
            value = trivial_phi(g, value);
        }
    }
    write_var(g, block, varid, value);
    return value;
}
void seal(IR g, i64 block) {
    index(g.blocks, block).sealed = true;
    i64 i = i64(0);
    while ((i < len(index(g.blocks, block).pending))) {
        i64 id = index(index(g.blocks, block).pending, i);
        i64 varid = parse_int(index(g.ins, id).text);
        i64 j = i64(0);
        while ((j < len(index(g.blocks, block).preds))) {
            i64 p = index(index(g.blocks, block).preds, j);
            push(index(g.ins, id).args, read_var(g, p, varid));
            push(index(g.ins, id).labels, p);
            j = wrap_add(j, i64(1));
        }
        trivial_phi(g, id);
        i = wrap_add(i, i64(1));
    }
}
void terminate(IR g, str term, Arr<i64> values, Arr<i64> targets) {
    i64 b = index(g.current, i64(0));
    index(g.blocks, b).term = term;
    index(g.blocks, b).values = values;
    index(g.blocks, b).targets = targets;
    i64 i = i64(0);
    while ((i < len(targets))) {
        if ((!has(index(g.blocks, index(targets, i)).preds, b))) {
            push(index(g.blocks, index(targets, i)).preds, b);
        }
        i = wrap_add(i, i64(1));
    }
}
i64 declare(IR g, str n, str ty, bool mut, i64 tok) {
    i64 i = wrap_sub(len(g.active), i64(1));
    while ((i >= i64(0))) {
        i64 id = index(g.active, i);
        if ((index(g.vars, id).depth != index(g.depth, i64(0)))) {
            break;
        }
        if ((index(g.vars, id).name == n)) {
            error(g.s, tok, wrap_add(str("duplicate local "), n));
        }
        i = wrap_sub(i, i64(1));
    }
    i64 id = len(g.vars);
    push(g.vars, Variable{n, ty, mut, index(g.depth, i64(0))});
    push(g.active, id);
    return id;
}
i64 lookup(IR g, str n) {
    i64 i = wrap_sub(len(g.active), i64(1));
    while ((i >= i64(0))) {
        if ((index(g.vars, index(g.active, i)).name == n)) {
            return index(g.active, i);
        }
        i = wrap_sub(i, i64(1));
    }
    return wrap_sub(0, i64(1));
}
void expect(IR g, str actual, str wanted, i64 tok) {
    if ((actual != wanted)) {
        error(g.s, tok,
              wrap_add(wrap_add(wrap_add(str("type mismatch: expected "), wanted), str(", got ")),
                       actual));
    }
}
i64 function_id(State s, str text, i64 owner) {
    str current = index(s.modules, owner);
    str wanted = current;
    str member = text;
    bool qualified_name = false;
    i64 k = i64(0);
    while ((wrap_add(k, i64(1)) < len(text))) {
        if ((slice(text, k, i64(2)) == str("::"))) {
            str alias = slice(text, i64(0), k);
            i64 j = i64(0);
            while ((j < len(s.imports))) {
                if (((index(s.imports, j).owner == owner) &&
                     (index(s.imports, j).alias == alias))) {
                    wanted = index(s.imports, j).module;
                    member =
                        slice(text, wrap_add(k, i64(2)), wrap_sub(wrap_sub(len(text), k), i64(2)));
                    qualified_name = true;
                }
                j = wrap_add(j, i64(1));
            }
            break;
        }
        k = wrap_add(k, i64(1));
    }
    i64 direct = map_get(s.symbols, wrap_add(wrap_add(wanted, str("::")), member));
    if ((direct >= i64(0))) {
        return direct;
    }
    if (qualified_name) {
        return wrap_sub(0, i64(1));
    }
    return map_get(s.symbols, wrap_add(str("*::"), member));
}
bool enum_type(State s, str t) {
    i64 r = record_id(s, t);
    return (((r >= i64(0)) && (len(index(s.records, r).types) == i64(1))) &&
            (index(index(s.records, r).types, i64(0)) == str("$enum")));
}
i64 field_id(IR g, str ty, str field, i64 tok) {
    i64 r = record_id(g.s, ty);
    if (((r < i64(0)) || enum_type(g.s, ty))) {
        error(g.s, tok, str("field access requires record"));
    }
    i64 i = find(index(g.s.records, r).names, field);
    if ((i < i64(0))) {
        error(g.s, tok, wrap_add(str("unknown field "), field));
    }
    return i;
}
str builtin_ret(str n) {
    if (((((((n == str("print")) || (n == str("eprint"))) || (n == str("println"))) ||
           (n == str("print_int"))) ||
          (n == str("println_int"))) ||
         (n == str("exit")))) {
        return str("void");
    }
    if (((((((n == str("read_file")) || (n == str("env"))) || (n == str("itoa"))) ||
           (n == str("chr"))) ||
          (n == str("slice"))) ||
         (n == str("join")))) {
        return str("str");
    }
    if (((((n == str("write_file")) || (n == str("run"))) || (n == str("parse_int"))) ||
         (n == str("clock_ns")))) {
        return str("i64");
    }
    if ((n == str("args"))) {
        return str("[]str");
    }
    return str("");
}
Arr<str> builtin_types(str n) {
    if (((((((n == str("print")) || (n == str("eprint"))) || (n == str("println"))) ||
           (n == str("read_file"))) ||
          (n == str("env"))) ||
         (n == str("parse_int")))) {
        return {str("str")};
    }
    if ((((((n == str("print_int")) || (n == str("println_int"))) || (n == str("exit"))) ||
          (n == str("itoa"))) ||
         (n == str("chr")))) {
        return {str("i64")};
    }
    if ((n == str("write_file"))) {
        return {str("str"), str("str")};
    }
    if ((n == str("slice"))) {
        return {str("str"), str("i64"), str("i64")};
    }
    if ((n == str("join"))) {
        return {str("[]str"), str("str")};
    }
    if ((n == str("run"))) {
        return {str("[]str")};
    }
    return {};
}
Value lower_call(IR g, Node n) {
    if ((index(g.s.nodes, n.a).tag != str("name"))) {
        error(g.s, n.tok, str("callee must name a function"));
    }
    str text = index(g.s.nodes, n.a).text;
    if ((((text == str("len")) || (text == str("push"))) || (text == str("pop")))) {
        i64 count = i64(1);
        if ((text == str("push"))) {
            count = i64(2);
        }
        if ((len(n.kids) != count)) {
            error(g.s, n.tok, str("wrong intrinsic arity"));
        }
        Value arr = lower_expr(g, index(n.kids, i64(0)), str(""));
        if (((!starts(arr.ty, str("[]"))) &&
             (!(((text == str("len")) && (arr.ty == str("str"))))))) {
            error(g.s, n.tok, str("intrinsic requires an array or string"));
        }
        if ((text == str("len"))) {
            return Value{emit(g, str("len"), str("i64"), arr.id, wrap_sub(0, i64(1)), {}, str("")),
                         str("i64")};
        }
        if ((text == str("pop"))) {
            return Value{emit(g, str("pop"), str("void"), arr.id, wrap_sub(0, i64(1)), {}, str("")),
                         str("void")};
        }
        Value value = lower_expr(g, index(n.kids, i64(1)), element(arr.ty));
        expect(g, value.ty, element(arr.ty), n.tok);
        return Value{emit(g, str("push"), str("void"), arr.id, value.id, {}, value.ty),
                     str("void")};
    }
    i64 fid = function_id(g.s, text, index(g.s.tokens, n.tok).file);
    str ret = str("");
    Arr<str> types = {};
    if ((fid >= i64(0))) {
        ret = index(g.s.functions, fid).ret;
        types = index(g.s.functions, fid).types;
    } else {
        ret = builtin_ret(text);
        types = builtin_types(text);
        if ((ret == str(""))) {
            error(g.s, n.tok, wrap_add(str("unknown or ambiguous function "), text));
        }
    }
    if ((len(types) != len(n.kids))) {
        error(g.s, n.tok, wrap_add(str("wrong number of arguments to "), text));
    }
    Arr<i64> values = {};
    i64 i = i64(0);
    while ((i < len(types))) {
        Value v = lower_expr(g, index(n.kids, i), index(types, i));
        expect(g, v.ty, index(types, i), n.tok);
        push(values, v.id);
        i = wrap_add(i, i64(1));
    }
    str target = text;
    if ((fid >= i64(0))) {
        target = wrap_add(str("@"), itoa(fid));
    }
    return Value{
        emit(g, str("call"), ret, wrap_sub(0, i64(1)), wrap_sub(0, i64(1)), values, target), ret};
}
Value lower_short(IR g, Node n) {
    Value left = lower_expr(g, n.a, str("bool"));
    expect(g, left.ty, str("bool"), n.tok);
    i64 from = index(g.current, i64(0));
    i64 rhs = new_block(g);
    i64 done = new_block(g);
    i64 shortvalue = i64(0);
    if ((n.text == str("||"))) {
        shortvalue = i64(1);
    }
    i64 init = constant(g, str("bool"), shortvalue);
    if ((n.text == str("&&"))) {
        terminate(g, str("branch"), {left.id}, {rhs, done});
    } else {
        terminate(g, str("branch"), {left.id}, {done, rhs});
    }
    seal(g, rhs);
    index(g.current, i64(0)) = rhs;
    Value right = lower_expr(g, n.b, str("bool"));
    expect(g, right.ty, str("bool"), n.tok);
    i64 last = index(g.current, i64(0));
    terminate(g, str("jump"), {}, {done});
    seal(g, done);
    index(g.current, i64(0)) = done;
    i64 phi = emit(g, str("phi"), str("bool"), wrap_sub(0, i64(1)), wrap_sub(0, i64(1)),
                   {init, right.id}, str(""));
    index(g.ins, phi).labels = {from, last};
    return Value{phi, str("bool")};
}
Value lower_expr_impl(IR g, i64 id, str hint) {
    Node n = index(g.s.nodes, id);
    i64 tok = n.tok;
    if ((n.tag == str("int"))) {
        str ty = str("i64");
        if ((hint == str("u8"))) {
            ty = str("u8");
        }
        i64 val = parse_int(n.text);
        if (((ty == str("u8")) && (((val < i64(0)) || (val > i64(255)))))) {
            error(g.s, tok, str("u8 literal out of range"));
        }
        return Value{constant(g, ty, val), ty};
    }
    if ((n.tag == str("bool"))) {
        i64 value = i64(0);
        if ((n.text == str("true"))) {
            value = i64(1);
        }
        return Value{constant(g, str("bool"), value), str("bool")};
    }
    if ((n.tag == str("string"))) {
        return Value{emit(g, str("string"), str("str"), wrap_sub(0, i64(1)), wrap_sub(0, i64(1)),
                          {}, n.text),
                     str("str")};
    }
    if ((n.tag == str("name"))) {
        i64 varid = lookup(g, n.text);
        if ((varid >= i64(0))) {
            return Value{read_var(g, index(g.current, i64(0)), varid), index(g.vars, varid).ty};
        }
        i64 fid = function_id(g.s, n.text, index(g.s.tokens, tok).file);
        if (((fid >= i64(0)) &&
             (index(g.s.nodes, index(g.s.functions, fid).body).tag == str("enumvalue")))) {
            return Value{constant(g, index(g.s.functions, fid).ret,
                                  parse_int(index(g.s.nodes, index(g.s.functions, fid).body).text)),
                         index(g.s.functions, fid).ret};
        }
        error(g.s, tok, wrap_add(str("undefined variable "), n.text));
    }
    if ((n.tag == str("call"))) {
        return lower_call(g, n);
    }
    if ((n.tag == str("array"))) {
        str ty = hint;
        Arr<i64> values = {};
        i64 i = i64(0);
        if ((!starts(ty, str("[]")))) {
            if ((len(n.kids) == i64(0))) {
                error(g.s, tok, str("empty array needs a type annotation"));
            }
            Value first = lower_expr(g, index(n.kids, i64(0)), str(""));
            ty = wrap_add(str("[]"), first.ty);
            push(values, first.id);
            i = i64(1);
        }
        i64 arr =
            emit(g, str("array"), ty, wrap_sub(0, i64(1)), wrap_sub(0, i64(1)), {}, element(ty));
        while ((i < len(n.kids))) {
            Value v = lower_expr(g, index(n.kids, i), element(ty));
            expect(g, v.ty, element(ty), tok);
            push(values, v.id);
            i = wrap_add(i, i64(1));
        }
        i = i64(0);
        while ((i < len(values))) {
            emit(g, str("push"), str("void"), arr, index(values, i), {}, element(ty));
            i = wrap_add(i, i64(1));
        }
        return Value{arr, ty};
    }
    if ((n.tag == str("record"))) {
        i64 rid = record_id(g.s, n.text);
        if (((rid < i64(0)) || enum_type(g.s, n.text))) {
            error(g.s, tok, wrap_add(str("unknown record "), n.text));
        }
        Record r = index(g.s.records, rid);
        if ((len(n.kids) != len(r.names))) {
            error(g.s, tok, str("record literal must initialize every field"));
        }
        Arr<i64> values = {};
        i64 i = i64(0);
        while ((i < len(r.names))) {
            push(values, wrap_sub(0, i64(1)));
            i = wrap_add(i, i64(1));
        }
        i = i64(0);
        while ((i < len(n.kids))) {
            Node field = index(g.s.nodes, index(n.kids, i));
            i64 fi = find(r.names, field.text);
            if ((fi < i64(0))) {
                error(g.s, field.tok, wrap_add(str("unknown field "), field.text));
            }
            if ((index(values, fi) >= i64(0))) {
                error(g.s, field.tok, str("duplicate initializer"));
            }
            Value v = lower_expr(g, field.a, index(r.types, fi));
            expect(g, v.ty, index(r.types, fi), field.tok);
            index(values, fi) = v.id;
            i = wrap_add(i, i64(1));
        }
        return Value{emit(g, str("record"), n.text, wrap_sub(0, i64(1)), wrap_sub(0, i64(1)),
                          values, str("")),
                     n.text};
    }
    if ((n.tag == str("index"))) {
        Value a = lower_expr(g, n.a, str(""));
        Value i = lower_expr(g, n.b, str("i64"));
        expect(g, i.ty, str("i64"), tok);
        if ((a.ty == str("str"))) {
            return Value{emit(g, str("byte"), str("u8"), a.id, i.id, {}, str("")), str("u8")};
        }
        if ((!starts(a.ty, str("[]")))) {
            error(g.s, tok, str("indexing requires array or string"));
        }
        return Value{emit(g, str("get"), element(a.ty), a.id, i.id, {}, str("")), element(a.ty)};
    }
    if ((n.tag == str("field"))) {
        Value a = lower_expr(g, n.a, str(""));
        i64 fi = field_id(g, a.ty, n.text, tok);
        str ty = index(index(g.s.records, record_id(g.s, a.ty)).types, fi);
        return Value{emit(g, str("extract"), ty, a.id, wrap_sub(0, i64(1)), {}, itoa(fi)), ty};
    }
    if ((n.tag == str("cast"))) {
        Value a = lower_expr(g, n.a, str(""));
        if (((!(((is_integer(a.ty) || (a.ty == str("bool"))) || enum_type(g.s, a.ty)))) ||
             (!(((is_integer(n.ty) || (n.ty == str("bool"))) || enum_type(g.s, n.ty)))))) {
            error(g.s, tok, str("invalid scalar cast"));
        }
        return Value{emit(g, str("cast"), n.ty, a.id, wrap_sub(0, i64(1)), {}, str("")), n.ty};
    }
    if ((n.tag == str("unary"))) {
        Value a = lower_expr(g, n.a, hint);
        if ((n.text == str("!"))) {
            expect(g, a.ty, str("bool"), tok);
            return Value{emit(g, str("not"), str("bool"), a.id, wrap_sub(0, i64(1)), {}, str("")),
                         str("bool")};
        }
        if ((!is_integer(a.ty))) {
            error(g.s, tok, str("integer operand required"));
        }
        str op = str("neg");
        if ((n.text == str("~"))) {
            op = str("inv");
        }
        return Value{emit(g, op, a.ty, a.id, wrap_sub(0, i64(1)), {}, str("")), a.ty};
    }
    if ((n.tag == str("binary"))) {
        if (((n.text == str("&&")) || (n.text == str("||")))) {
            return lower_short(g, n);
        }
        Value a = lower_expr(g, n.a, str(""));
        Value b = lower_expr(g, n.b, a.ty);
        expect(g, b.ty, a.ty, tok);
        if (((n.text == str("+")) && (a.ty == str("str")))) {
            return Value{emit(g, str("concat"), str("str"), a.id, b.id, {}, str("")), str("str")};
        }
        i64 opi = find({str("+"), str("-"), str("*"), str("/"), str("%"), str("<<"), str(">>"),
                        str("&"), str("|"), str("^"), str("=="), str("!="), str("<"), str("<="),
                        str(">"), str(">=")},
                       n.text);
        Arr<str> ops = {str("add"), str("sub"), str("mul"), str("div"), str("rem"), str("shl"),
                        str("shr"), str("and"), str("or"),  str("xor"), str("eq"),  str("ne"),
                        str("lt"),  str("le"),  str("gt"),  str("ge")};
        if ((opi < i64(0))) {
            error(g.s, tok, str("unknown binary operator"));
        }
        str ty = a.ty;
        if ((opi >= i64(10))) {
            ty = str("bool");
        }
        if (((opi < i64(10)) && (!is_integer(a.ty)))) {
            error(g.s, tok, str("integer operands required"));
        }
        if (((opi >= i64(12)) && (!is_integer(a.ty)))) {
            error(g.s, tok, str("ordered comparison requires integers"));
        }
        if ((((opi >= i64(10)) && (opi < i64(12))) &&
             (!((((is_integer(a.ty) || (a.ty == str("str"))) || (a.ty == str("bool"))) ||
                 enum_type(g.s, a.ty)))))) {
            error(g.s, tok, str("equality requires scalar operands"));
        }
        return Value{emit(g, index(ops, opi), ty, a.id, b.id, {}, str("")), ty};
    }
    error(g.s, tok, wrap_add(str("invalid expression "), n.tag));
    return Value{wrap_sub(0, i64(1)), str("void")};
}
Value lower_expr(IR g, i64 id, str hint) {
    Value v = lower_expr_impl(g, id, hint);
    index(g.s.nodes, id).ty = v.ty;
    return v;
}
Value lvalue(IR g, i64 id, Arr<Lens> path) {
    Node n = index(g.s.nodes, id);
    if ((n.tag == str("name"))) {
        i64 v = lookup(g, n.text);
        if ((v < i64(0))) {
            error(g.s, n.tok, wrap_add(str("undefined variable "), n.text));
        }
        push(path,
             Lens{str("var"), index(g.vars, v).ty, v, wrap_sub(0, i64(1)), wrap_sub(0, i64(1))});
        return Value{read_var(g, index(g.current, i64(0)), v), index(g.vars, v).ty};
    }
    if ((n.tag == str("index"))) {
        Value a = lower_expr(g, n.a, str(""));
        if ((!starts(a.ty, str("[]")))) {
            error(g.s, n.tok, str("assignment index requires mutable array"));
        }
        Value index = lower_expr(g, n.b, str("i64"));
        expect(g, index.ty, str("i64"), n.tok);
        while ((len(path) > i64(0))) {
            pop(path);
        }
        str ty = element(a.ty);
        push(path, Lens{str("index"), ty, a.id, index.id, wrap_sub(0, i64(1))});
        return Value{emit(g, str("get"), ty, a.id, index.id, {}, str("")), ty};
    }
    if ((n.tag == str("field"))) {
        Value a = lvalue(g, n.a, path);
        i64 f = field_id(g, a.ty, n.text, n.tok);
        str ty = index(index(g.s.records, record_id(g.s, a.ty)).types, f);
        push(path, Lens{str("field"), ty, a.id, wrap_sub(0, i64(1)), f});
        return Value{emit(g, str("extract"), ty, a.id, wrap_sub(0, i64(1)), {}, itoa(f)), ty};
    }
    error(g.s, n.tok, str("invalid assignment target"));
    return Value{wrap_sub(0, i64(1)), str("void")};
}
void lower_statement(IR g, i64 id) {
    Node n = index(g.s.nodes, id);
    i64 tok = n.tok;
    if ((n.tag == str("block"))) {
        i64 saved = len(g.active);
        index(g.depth, i64(0)) = wrap_add(index(g.depth, i64(0)), i64(1));
        i64 i = i64(0);
        bool unreachable = false;
        while ((i < len(n.kids))) {
            if ((index(g.current, i64(0)) < i64(0))) {
                unreachable = true;
                i64 ghost = new_block(g);
                index(g.blocks, ghost).alive = false;
                index(g.current, i64(0)) = ghost;
            }
            lower_statement(g, index(n.kids, i));
            i = wrap_add(i, i64(1));
        }
        while ((len(g.active) > saved)) {
            pop(g.active);
        }
        index(g.depth, i64(0)) = wrap_sub(index(g.depth, i64(0)), i64(1));
        if (unreachable) {
            index(g.current, i64(0)) = wrap_sub(0, i64(1));
        }
        return;
    }
    if ((n.tag == str("decl"))) {
        Value v = lower_expr(g, n.a, n.ty);
        if ((n.ty != str(""))) {
            if ((!known_type(g.s, n.ty))) {
                error(g.s, tok, wrap_add(str("unknown type "), n.ty));
            }
            expect(g, v.ty, n.ty, tok);
        }
        if ((v.ty == str("void"))) {
            error(g.s, tok, str("cannot bind void"));
        }
        index(g.s.nodes, id).ty = v.ty;
        i64 varid = declare(g, n.text, v.ty, (n.b == i64(1)), tok);
        write_var(g, index(g.current, i64(0)), varid, v.id);
        return;
    }
    if ((n.tag == str("expr"))) {
        lower_expr(g, n.a, str(""));
        return;
    }
    if ((n.tag == str("assign"))) {
        Arr<Lens> path = {};
        Value target = lvalue(g, n.a, path);
        Value rhs = lower_expr(g, n.b, target.ty);
        expect(g, rhs.ty, target.ty, tok);
        i64 value = rhs.id;
        i64 i = wrap_sub(len(path), i64(1));
        while ((i >= i64(0))) {
            Lens lens = index(path, i);
            if ((lens.kind == str("var"))) {
                if ((!index(g.vars, lens.base).writable)) {
                    error(g.s, tok, str("cannot assign to let binding"));
                }
                write_var(g, index(g.current, i64(0)), lens.base, value);
            } else {
                if ((lens.kind == str("index"))) {
                    emit(g, str("set"), str("void"), lens.base, lens.index, {value}, lens.ty);
                    return;
                } else {
                    value = emit(g, str("insert"), index(g.ins, canonical(g, lens.base)).ty,
                                 lens.base, value, {}, itoa(lens.field));
                }
            }
            i = wrap_sub(i, i64(1));
        }
        return;
    }
    if ((n.tag == str("return"))) {
        str ret = index(g.s.functions, g.fid).ret;
        Arr<i64> values = {};
        if ((n.a >= i64(0))) {
            Value v = lower_expr(g, n.a, ret);
            expect(g, v.ty, ret, tok);
            if ((ret == str("void"))) {
                error(g.s, tok, str("void function cannot return a value"));
            }
            push(values, v.id);
        } else {
            expect(g, str("void"), ret, tok);
        }
        terminate(g, str("return"), values, {});
        index(g.current, i64(0)) = wrap_sub(0, i64(1));
        return;
    }
    if (((n.tag == str("break")) || (n.tag == str("continue")))) {
        if ((len(g.breaks) == i64(0))) {
            error(g.s, tok, wrap_add(n.tag, str(" outside a loop")));
        }
        i64 target = index(g.breaks, wrap_sub(len(g.breaks), i64(1)));
        if ((n.tag == str("continue"))) {
            target = index(g.continues, wrap_sub(len(g.continues), i64(1)));
        }
        terminate(g, str("jump"), {}, {target});
        index(g.current, i64(0)) = wrap_sub(0, i64(1));
        return;
    }
    if ((n.tag == str("if"))) {
        Value cond = lower_expr(g, n.a, str("bool"));
        expect(g, cond.ty, str("bool"), tok);
        i64 yes = new_block(g);
        i64 no = new_block(g);
        i64 done = new_block(g);
        terminate(g, str("branch"), {cond.id}, {yes, no});
        seal(g, yes);
        seal(g, no);
        index(g.current, i64(0)) = yes;
        lower_statement(g, n.b);
        if ((index(g.current, i64(0)) >= i64(0))) {
            terminate(g, str("jump"), {}, {done});
        }
        index(g.current, i64(0)) = no;
        if ((n.c >= i64(0))) {
            lower_statement(g, n.c);
        }
        if ((index(g.current, i64(0)) >= i64(0))) {
            terminate(g, str("jump"), {}, {done});
        }
        seal(g, done);
        if ((len(index(g.blocks, done).preds) == i64(0))) {
            index(g.blocks, done).alive = false;
            index(g.current, i64(0)) = wrap_sub(0, i64(1));
        } else {
            index(g.current, i64(0)) = done;
        }
        return;
    }
    if ((n.tag == str("while"))) {
        i64 header = new_block(g);
        i64 body = new_block(g);
        i64 done = new_block(g);
        terminate(g, str("jump"), {}, {header});
        index(g.current, i64(0)) = header;
        Value cond = lower_expr(g, n.a, str("bool"));
        expect(g, cond.ty, str("bool"), tok);
        terminate(g, str("branch"), {cond.id}, {body, done});
        seal(g, body);
        push(g.breaks, done);
        push(g.continues, header);
        index(g.current, i64(0)) = body;
        lower_statement(g, n.b);
        if ((index(g.current, i64(0)) >= i64(0))) {
            terminate(g, str("jump"), {}, {header});
        }
        pop(g.breaks);
        pop(g.continues);
        seal(g, header);
        seal(g, done);
        index(g.current, i64(0)) = done;
        return;
    }
    error(g.s, tok, wrap_add(str("unknown statement "), n.tag));
}
void check_layout(State s, str ty, Arr<str> stack, i64 tok) {
    if ((!known_type(s, ty))) {
        error(s, tok, wrap_add(str("unknown type "), ty));
    }
    if (((starts(ty, str("[]")) || (record_id(s, ty) < i64(0))) || enum_type(s, ty))) {
        return;
    }
    if ((find(stack, ty) >= i64(0))) {
        error(s, tok, wrap_add(str("infinitely recursive value record "), ty));
    }
    push(stack, ty);
    Record r = index(s.records, record_id(s, ty));
    i64 i = i64(0);
    while ((i < len(r.types))) {
        if ((index(r.types, i) == str("void"))) {
            error(s, tok, str("void record field"));
        }
        check_layout(s, index(r.types, i), stack, tok);
        i = wrap_add(i, i64(1));
    }
    pop(stack);
}
IR lower_function(State s, i64 fid) {
    Function f = index(s.functions, fid);
    IR g = new_ir(s, fid);
    new_block(g);
    seal(g, i64(0));
    i64 i = i64(0);
    check_layout(s, f.ret, {}, f.tok);
    while ((i < len(f.names))) {
        check_layout(s, index(f.types, i), {}, f.tok);
        if ((index(f.types, i) == str("void"))) {
            error(s, f.tok, str("void parameter"));
        }
        i64 v = declare(g, index(f.names, i), index(f.types, i), false, f.tok);
        write_var(g, i64(0), v,
                  emit(g, str("arg"), index(f.types, i), wrap_sub(0, i64(1)), wrap_sub(0, i64(1)),
                       {}, itoa(i)));
        i = wrap_add(i, i64(1));
    }
    lower_statement(g, f.body);
    if ((index(g.current, i64(0)) >= i64(0))) {
        if ((f.ret != str("void"))) {
            error(s, f.tok, wrap_add(str("missing return in function "), f.name));
        }
        terminate(g, str("return"), {}, {});
    }
    bool change = true;
    while (change) {
        change = false;
        i = i64(0);
        while ((i < len(g.ins))) {
            if (((index(g.ins, i).op == str("phi")) && (index(g.ins, i).repl < i64(0)))) {
                if ((trivial_phi(g, i) != i)) {
                    change = true;
                }
            }
            i = wrap_add(i, i64(1));
        }
    }
    Arr<i64> reached = {};
    i = i64(0);
    while ((i < len(g.blocks))) {
        push(reached, i64(0));
        i = wrap_add(i, i64(1));
    }
    Arr<i64> work = {i64(0)};
    while ((len(work) > i64(0))) {
        i64 b = index(work, wrap_sub(len(work), i64(1)));
        pop(work);
        if ((index(reached, b) == i64(0))) {
            index(reached, b) = i64(1);
            i = i64(0);
            while ((i < len(index(g.blocks, b).targets))) {
                push(work, index(index(g.blocks, b).targets, i));
                i = wrap_add(i, i64(1));
            }
        }
    }
    i = i64(0);
    while ((i < len(g.blocks))) {
        index(g.blocks, i).alive = (index(reached, i) == i64(1));
        i = wrap_add(i, i64(1));
    }
    return g;
}
int main(int argc, char **argv) {
    for (int i = 0; i < argc; i++)
        push(seed_args, str(argv[i]));
    return (int)aster_main();
}
