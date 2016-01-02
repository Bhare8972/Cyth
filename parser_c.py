
from lexer import logger, default_error, location_span, lexer_generator

## TODO change adding lexer actions, so that the terminal is called, and returns the action  
##using epsilon terminal does not work 
        
##### languege definition classes
token_static_id=0
class token(object):
    """represents a symbol in the languege"""
    def __init__(self, name, is_terminal):
        if name==None:
            name='unnamed'
        self.name=name
        self.is_terminal=is_terminal
        global token_static_id
        self.id=token_static_id
        token_static_id+=1
        
    def __str__(self):
        return self.name
        
    def __repr__(self):
        return self.name
        
class terminal_definition(token):
    """represents a token that is a terminal"""
    def __init__(self, name, action=None):
        token.__init__(self, name, True)
        self.error_recovery_terminals=[]
        self.action=action
        
    def __call__(self, data, location_span):
        if self.action:
            data=self.action(data)
        return token_data(self.id, data, location_span)
            
    def add_error_terms(self, *args):
        self.error_recovery_terminals+=args
        
    def __str__(self):
        return self.name
            
    
class nonterm_definition(token):
    """represents a token that is not a terminal"""
    def __init__(self, name, parser_generator):
        token.__init__(self, name, False)
        self.productions=[]
        self.parser_generator=parser_generator
        
    def add_production(self, *args, **kwargs):
        tokens=[]
        for t in args:
            if type(t)==str:
                t=self.parser_generator.terminal_to_lexer(t)
            tokens.append(t)
        action=None
        if 'action' in kwargs:
            action=kwargs['action']
            
        if len(tokens)==0:
            print "ERROR: no tokens in production"
            return
        new_prod=production(self, tokens, action)
        self.productions.append( new_prod  )
        return new_prod

production_static_id=0
class production(object):
    """represents one production in the grammer"""
    def __init__(self, Lval, tokens, action):
        self.Lval=Lval
        self.tokens=tokens
        self.action=action
        global production_static_id
        self.id=production_static_id
        production_static_id+=1
        self.assoc='none'
        
    def get_info(self):
        return production_info(self.Lval.id, self.Lval.name, len(self.tokens), self.action)
        
    def __str__(self):
        out=[self.Lval.name, '->']
        for token in self.tokens:
            out.append(token.name)
        return ' '.join(out)
        
    def set_assoc(self, set_assoc):
        if set_assoc in ['none', 'left', 'right']:
            self.assoc=set_assoc
        else:
            print "ERROR: associativity should be 'none', 'left', or 'right'"
        
####grammer data used by final parser
class token_data(object):
    """represents a token and data ascociated with ih"""
    def __init__(self, id, data, span):
        self.id=id
        self.data=data
        self.span=span
        
class production_info(object):
    """represents basic information about a production, that is usefull to the final parser"""
    def __init__(self, Lval_id, Lval_name, n_tokens, action):
        self.Lval_id=Lval_id
        self.Lval_name=Lval_name
        self.n_tokens=n_tokens
        self.action=action
        
    def __str__(self):
        return self.Lval_name+':'+str(self.n_tokens)
        
#### classes used to process the grammer
class item(object):
    """represents a production, and a parser location in the production"""
    def __init__(self, production, location, lookahead=None):
        self.production=production
        self.location=location
        self.lookahead=lookahead
        
    def is_kernel(self, augumented_start):
        if self.location!=0:
            return True
        if self.production is augumented_start.productions[0]:
            return True
        return False
        
    def is_LR0_item(self):
        return self.lookahead is None
    
    def get_postTokens(self):
        return self.production.tokens[self.location:]
        
    def copy(self, lookahead=False):
        if lookahead==False:
            lookahead=self.lookahead
        ret=item(self.production, self.location, lookahead)
        return ret
        
    def __eq__(self, RHS):
        return self.production==RHS.production and self.location==RHS.location and self.lookahead==RHS.lookahead
        
    def __str__(self):
        ret=[str(self.production.Lval.name)]
        ret.append('->')
        for i in xrange(len(self.production.tokens)):
            if i==self.location:
                ret.append('.')
            ret.append(self.production.tokens[i].name)
            ret.append(' ')
        if self.location==len(self.production.tokens):
            ret.append('.')
        if self.lookahead:
            ret.append('[')
            ret.append(self.lookahead.name)
            ret.append(']')
        
        return ''.join(ret)
        
class item_set(object):
    def __init__(self, id=None, first_item=None):
        self.items=[]
        self.goto_table={}
        self.id=id
        if first_item!=None:
            self.items.append(first_item)
        
    def append(self, item):
        self.items.append(item)
        
    def add_goto(self, token, item_set):
        self.goto_table[token.id]=item_set
        
    def goto(self, token):
        if token.id not in self.goto_table:
            return None
        return self.goto_table[token.id]
        
    def __eq__(self, RHS):
        if RHS==None:
            return False
        return self.items==RHS.items
        
    def __iter__(self):
        return self.items.__iter__()
        
    def __len__(self):
        return len(self.items)
        
    def __str__(self):
        out=['SET: ']
        out.append(str(self.id))
        out.append('\n')
        for item in self.items:
            out.append('  '+str(item))
            out.append('\n')
        out.append('END SET')
        return ''.join(out)

        
class propagation_table(object):
    def __init__(self, all_item_sets):
        self.all_item_sets=all_item_sets
        self.table={}
        
    def add_propagation(self, from_set, from_item, to_set, to_item):
        from_tuple=(from_set.id, from_set.items.index(from_item))
        to_tuple=(to_set, to_item)
        if from_tuple in self.table:
            self.table[from_tuple].append( to_tuple )
        else:
            self.table[from_tuple]=[ to_tuple ]
            
    def get_propagations(self, from_set, from_item):
        from_tuple=(from_set.id, from_set.items.index(from_item))
        if from_tuple not in self.table:
            return []
        to_tuples=self.table[from_tuple]
        return to_tuples
    
    def __str__(self):
        out=[]
        for from_tuple, to_tuples in self.table.iteritems():
            for set in self.all_item_sets:
                if set.id==from_tuple[0]:
                    item=set.items[from_tuple[1]]
                    
            out.append("FROM: [")
            out.append(str(from_tuple[0]))
            out.append("]  ")
            out.append(str(item))
            out.append("\n")
            out.append("TO:\n")
            
            for to_set, to_item in to_tuples:
                out.append("   [")
                out.append(str(to_set.id))
                out.append("]  ")
                out.append(str(to_item))
                out.append("\n")
            out.append('\n')
        return ''.join(out)
            
                    
            
                
####tools used to represent states in parser
class action(object):
    @staticmethod
    def error():
        return action('e')
        
    @staticmethod
    def accept():
        return action('a')
        
    @staticmethod
    def shift(state):
        return action('s',state)
        
    @staticmethod
    def reduce(production):
        return action('r', production)
    
    def __init__(self, sym, arg=None):
        self.symbol=sym
        self.argument=arg
        
    def is_error(self):
        return self.symbol=='e'
        
    def is_reduce(self):
        return self.symbol=='r'
        
    def is_shift(self):
        return self.symbol=='s'
        
    def is_accept(self):
        return self.symbol=='a'
        
    def __str__(self):
        if self.symbol=='a':
            return "accept"
        elif self.symbol=='s':
            return "shift to "+str(self.argument)
        elif self.symbol=='r':
            return "reduce by "+str(self.argument)
        elif self.symbol=='e':
            return "error"
            
    def __eq__(self, RHS):
        if RHS==None:
            return False
        return self.symbol==RHS.symbol and self.argument==RHS.argument

class state(object):
    def __init__(self):
        self.ACTION={}
        self.GOTO={}
        self.default_action=action.error()
        
    def add_action(self, term, action):
        self.ACTION[term]=action
        
    def add_goto(self, nonterm, state):
        self.GOTO[nonterm]=state
        
    def add_default(self, action):
        self.default_action=action
        
    def goto(self, nonterm):
        return self.GOTO[nonterm]
        
    def action(self, term):
        if term in self.ACTION:
            return self.ACTION[term]
        else:
            return self.default_action
            
#### parser and parser generator
class parser_generator(object):
    def __init__(self, lex_gen):
        self.EOF_terminal=terminal_definition("EOF")
        self.EPSILON_terminal=terminal_definition("epsilon")
        self.terminals={"EOF":self.EOF_terminal, "epsilon":self.EPSILON_terminal}
        self.nonterms=[]
        self.lex_gen=lex_gen
        self.error_function=default_error
        self.start_nonterm=None
        self.opperator_precedence=[]
        
        self.parser_info_generated=False        

#### define grammer        
#    def set_precidance(*args):
#        stuff
        
    def set_error_function(self,erf):
        self.error_function=erf
        
    def get_EOF_term(self):
        return self.EOF_terminal
    
    def get_EPSILON_term(self):
        return self.EPSILON_terminal
        
    def new_terminal(self, name):
        TT=terminal_definition(name)
        self.terminals[name]=TT
        return TT
        
    def new_nonterm(self, name=None):
        NT=nonterm_definition(name, self)
        self.nonterms.append(NT)
        if self.start_nonterm==None:
            self.start_nonterm=NT
        return NT
        
    def set_start_nonterm(self, non_term):
        self.start_nonterm=non_term
        
    def print_grammer(self):
        for non_term in self.nonterms:
            print non_term.name
            prefix=(len(non_term.name)+1)*" "+" |"
            for production in non_term.productions:
                print prefix,
                for token in production.tokens:
                    print token.name,
                print
            print
            
    def set_precedence(self, *args):
        self.opperator_precedence+=args
        
            
    ## internal use to add new terminals
    def terminal_to_lexer(self, name):
        if name in self.terminals:
            return self.terminals[name]
        else:
            term=self.new_terminal(name)
            self.lex_gen.add_pattern('"'+name+'"', term)
            return term
        
#### algorithms to generate parser
                
    def first(self, tokens):
        ##may be a simpler algorithm        
        
        ret=[]
        has_epsilon=False
        all_prev_have_epsilon=True
        for token in tokens:
            toke_first, toke_epsilon=self.first_single(token)
            ret+=toke_first
            has_epsilon=has_epsilon or toke_epsilon
            if not toke_epsilon:
                all_prev_have_epsilon=False
                break
        if all_prev_have_epsilon:
            has_epsilon=True
            
        ##remove duplicates
        new_ret=[]
        for i in xrange(len(ret)):
            f=ret[0]
            ret=ret[1:]
            if f not in ret:
                new_ret.append(f)
        if has_epsilon:
            new_ret.append(self.EPSILON_terminal)
            
        return new_ret
        
    def first_single(self, token, exclusion_productions=[]):
        if token.is_terminal and token==self.EPSILON_terminal:
            return [], True
        elif token.is_terminal and token!=self.EPSILON_terminal:
            return [token], False
        else:
            ret=[]
            has_epsilon=False
            for p in token.productions:
                if p.id in exclusion_productions:
                    continue
                prod_tokens=p.tokens
                all_prev_had_epsilon=True
                for p_toke in prod_tokens:
                    add_excl=[p.id] if p_toke==token else []
                    p_toke_first,p_toke_epsilon=self.first_single(p_toke, exclusion_productions+add_excl)
                    if all_prev_had_epsilon:
                        ret+=p_toke_first
                        has_epsilon = has_epsilon or p_toke_epsilon
                    if not p_toke_epsilon:
                        all_prev_had_epsilon=False
                        break
                if all_prev_had_epsilon:
                    has_epsilon=True
            return ret, has_epsilon
        
    def closure_LR1(self, input_set):
        items_added=True
        while items_added:
            items_added=False
            for input_item in input_set:
                post_tokens=input_item.get_postTokens()
                if len(post_tokens)>0 and not post_tokens[0].is_terminal:
                    next_token=post_tokens[0]
                    following_tokens=post_tokens[1:]+[input_item.lookahead]
                    first_data=self.first(following_tokens)
                    for token_production in next_token.productions:
                        for first_terminal in first_data:
                            new_item=item(token_production, 0, first_terminal)
                            if new_item not in input_set.items:
                                input_set.append( new_item )
                                items_added=True
                    
    def closure_LR0(self, input_set):
        items_added=True
        while items_added:
            items_added=False
            for input_item in input_set:
                post_tokens=input_item.get_postTokens()
                if len(post_tokens)>0 and not post_tokens[0].is_terminal:
                    next_token=post_tokens[0]
                    for next_token_production in next_token.productions:
                        prod_item=item(next_token_production,0)
                        if not prod_item in input_set.items:
                            input_set.append( prod_item )
                            items_added=True
        
    def goto_LR0(self, input_set, token):
        ret=item_set()
        for input_item in input_set:
            post_tokens=input_item.get_postTokens()
            if len(post_tokens)>0 and post_tokens[0]==token:
                new_item=input_item.copy()
                new_item.location+=1
                ret.append(new_item) 
            
        self.closure_LR0(ret)
        return ret
        
    def LR0_itemsets(self, augmented_start):
        set_zero=item_set(id=0)
        start_item=item(augmented_start.productions[0], 0)
        set_zero.append(start_item)
        self.closure_LR0(set_zero)
        item_sets=[ set_zero ]
        next_set_id=1
        
#        print "NEW SET:",set_zero.id
#        print set_zero
#        print
#            
        for set in item_sets:
            
            for term in self.terminals.itervalues():
                new_goto=self.goto_LR0(set, term)
                if len(new_goto)>0 and (not new_goto in item_sets):
                    item_sets.append(new_goto)
                    new_goto.id=next_set_id
                    next_set_id+=1
                    set.add_goto(term, new_goto)

#                    print "NEW SET:",new_goto.id  
#                    print new_goto
#                    print
#                    print "NEW GOTO: FROM",set.id," to ",new_goto.id," on ",term.name
#                    print
                    
                elif new_goto in item_sets:
                    new_goto=item_sets[ item_sets.index(new_goto)  ]
                    set.add_goto(term, new_goto)
#                    print "NEW GOTO: FROM",set.id," to ",new_goto.id," on ",term.name
#                    print
                    
            for nonterm in self.nonterms:
                new_goto=self.goto_LR0(set, nonterm)
                if len(new_goto)>0 and (not new_goto in item_sets):
                    item_sets.append(new_goto)
                    new_goto.id=next_set_id
                    next_set_id+=1
                    set.add_goto(nonterm, new_goto)

#                    print "NEW SET:",new_goto.id
#                    print new_goto
#                    print
#                    print "NEW GOTO: FROM",set.id," to ",new_goto.id," on ",nonterm.name
#                    print
                elif new_goto in item_sets:
                    new_goto=item_sets[ item_sets.index(new_goto)  ]
                    set.add_goto(nonterm, new_goto)
#                    print "NEW GOTO: FROM",set.id," to ",new_goto.id," on ",nonterm.name
#                    print
                        
        return item_sets
        
    def generate_parser_tables(self, log):
        ##initializations
        self.lex_gen.set_EOF_action( self.EOF_terminal )
        ##augment grammer
        if not self.start_nonterm:
            print "need at least one production"
            return
        augmented_start=self.new_nonterm('StartToken')
        augmented_start.add_production(self.start_nonterm)
        
        #### basic accounting
        ##generate the terminal map.  Map from the terminal index to the terminal name
        self.term_map={}#['']*len(self.terminals)
        log('terminals:')
        for term in self.terminals.itervalues():
            log(term.id, ': ', term.name)
            self.term_map[term.id]=term.name
        log()            
            
        ##generate terminal error recovery table
        self.terminal_error_recovery_table=[]
        for term in self.terminals.itervalues():
            terminal_recovery=[]
            for rec_term in term.error_recovery_terminals:
                terminal_recovery.append(rec_term.id)
            self.terminal_error_recovery_table.append(terminal_recovery)
            
        ##log the nonterms
        log('nonterminals:')
        for nonterm in self.nonterms:
            log(nonterm.id, ': ', nonterm.name)
        log()            
            
            
        ##set the basic production information
        ##productions are not necissarily in order...oddly enough
        N_productions=0    
        for nonterm in self.nonterms:   
            N_productions+=len(nonterm.productions)
            
        self.production_info_list=[None]*N_productions
        log('grammer productions:')
        nonterm_map={}
        for nonterm in self.nonterms:
            nonterm_map[nonterm.id]=nonterm.name
            for p in nonterm.productions:
                log(p.id,': ',p)
                self.production_info_list[p.id]=p.get_info()
        log()
            
        
            
        ####generate the parse table!
        ##generate all LR0 sets of items
        LR0_item_sets=self.LR0_itemsets(augmented_start)     
        
        ##remove kernal items
        for set in LR0_item_sets:
            for i in xrange(len(set)-1,-1,-1):
                if not set.items[i].is_kernel( augmented_start):
                    del set.items[i]
                    
        ##algorithm 4.62 generate propagation table and spontanious lookaheads
        ##create the new LR1 item sets from the old LR0 item sets
        ##maybe we want to put the LR1 sets in a dictionary?
        LR1_item_sets=[ item_set(s.id) for s in LR0_item_sets ] ##the sets themselves are the same
        for LR1_set,LR0_set in zip(LR1_item_sets, LR0_item_sets):
            ##duplicate the goto table from LR0_set to LR1_set
            for goto_token_ID,LR0_goto_set in LR0_set.goto_table.iteritems():
                LR1_goto_set=LR1_item_sets[ LR0_goto_set.id ]
                LR1_set.goto_table[goto_token_ID]=LR1_goto_set
        
        #### create the propagation table and the spontiaously generated lookaheads
        prop_table=propagation_table(LR0_item_sets)
        fake_terminal=terminal_definition(None)
        for set_K in LR0_item_sets:
            for token_X in self.terminals.values() + self.nonterms:
                
                LR0_goto_set=set_K.goto(token_X)
                if LR0_goto_set==None:
                    continue
                LR1_set=LR1_item_sets[ LR0_goto_set.id ]
            
                for K_item in set_K:
                    set_J=item_set(id=0, first_item=K_item.copy(fake_terminal) )
                    self.closure_LR1( set_J )
                    
                    for J_item in set_J:
                        post_tokens=J_item.get_postTokens()
                        if len(post_tokens)>0 and post_tokens[0]==token_X:
                            J_item_copy=J_item.copy()
                            J_item_copy.location+=1
                            if J_item.lookahead==fake_terminal:  ##all lookaheads propagate from K_item to J_item
                                J_item_copy.lookahead=None
                                prop_table.add_propagation(set_K, K_item, LR1_set, J_item_copy)
                            else:  ##j_item lookhead is spontaniously generated
                                if  not J_item_copy in LR1_set.items:
                                    LR1_set.append(J_item_copy)
        ## add EOF to augmented start production
        for LR0_item in LR0_item_sets[0]:
            if LR0_item.production==augmented_start.productions[0] and LR0_item.location==0:
                new_item=LR0_item.copy(self.EOF_terminal)
                LR1_item_sets[0].append(new_item)
                        
        ##propagate lookahead items to form all LALR(1) items
        items_added=True
        while items_added:
            items_added=False
            
            for  LR1set in LR1_item_sets:
                LR0_set=LR0_item_sets[ LR1set.id ]
                for LR1item in LR1set:
                    LR0item=LR1item.copy(None)
                    propagate_to=prop_table.get_propagations(LR0_set,LR0item )
                    for to_set, to_item in propagate_to:
                        new_to_item=to_item.copy( LR1item.lookahead )
                        if new_to_item not in to_set:
                            to_set.append(new_to_item)
                            items_added=True
                
        
        ##closure on each set of items to get non-kernal items
        [ self.closure_LR1(set) for set in LR1_item_sets ]

        ##generate table using algoritm 4.56
        self.states=[ state() for x in LR1_item_sets] ## a state for each item set
        accept_item=item(augmented_start.productions[0], 1, self.EOF_terminal)
        is_ambiguous=False
        for state_i in xrange(len(self.states)):
            current_state=self.states[state_i]
            set=LR1_item_sets[state_i]
            ##set the actions
            new_action=None
            action_items={}
            for set_item in set:
                post_tokens=set_item.get_postTokens()
                if set_item==accept_item:
                    ##accept
                    new_action=action.accept()
                    action_token=self.EOF_terminal
                elif len(post_tokens)==0 and set_item.lookahead.is_terminal:
                    ##reduce
                    new_action=action.reduce(set_item.production.id)
                    action_token=set_item.lookahead
                elif post_tokens[0].is_terminal:
                    ##shift
                    set_to_shift_to=set.goto(post_tokens[0])
                    new_action=action.shift(set_to_shift_to.id)
                    action_token=post_tokens[0]
                
                if new_action:
                    tmp_ambiguous=False
                    ##check for conflicts
                    old_action=current_state.action(action_token.id)
                    if (not old_action.is_error()) and not old_action==new_action:
                        tmp_ambiguous=True
                        ##check to see if conflict can be solved by associativity and precidance
                        if (old_action.is_shift() and new_action.is_reduce()) or (new_action.is_shift() and old_action.is_reduce()): ##make sure we have a shift-reduce conflict
                            ##order the shift and reduce actions
                            if old_action.is_shift():
                                shift_action=old_action
                                reduce_action=new_action
                                reduce_item=set_item
                                shift_item=action_items[action_token.id]
                            else:
                                shift_action=new_action,
                                reduce_action=old_action
                                shift_item=set_item
                                reduce_item=action_items[action_token.id]
                                
                            if reduce_item.production==shift_item.production: ##try associativity
                                if reduce_item.production.assoc=='left':
                                    new_action=reduce_action
                                    set_item=reduce_item
                                    tmp_ambiguous=False
                                elif reduce_item.production.assoc=='right':
                                    new_action=shift_action
                                    set_item=shift_item
                                    tmp_ambiguous=False
                                    
                            elif (reduce_item.production in self.opperator_precedence) and (shift_item.production in self.opperator_precedence): ##try precidance
                                reduce_index=self.opperator_precedence.index(reduce_item.production)
                                shift_index=self.opperator_precedence.index(shift_item.production)
                                if shift_index<reduce_index:
                                    new_action=shift_action
                                    set_item=shift_item
                                    tmp_ambiguous=False
                                elif reduce_index<shift_index:
                                    new_action=reduce_action
                                    set_item=reduce_item
                                    tmp_ambiguous=False
                                    
                        if tmp_ambiguous:
                            ##not able to resolve ambiguity
                            log("AMBIGUOUS GRAMMER in state ",state_i,". conflict between action: ", old_action,
                                " on item:",action_items[action_token.id], ", and action: ",new_action," on item:",set_item)
                            is_ambiguous=True ##we do not return here, becouse we want to list out all ambiguities
                    
                    if not tmp_ambiguous:
                        current_state.add_action(action_token.id, new_action)
                        action_items[action_token.id]=set_item
                        
            ##set the goto table
            for nonterm in self.nonterms:
                goto=set.goto(nonterm)
                if goto:
                    current_state.add_goto(nonterm.id, goto.id)
        
        ##log action table
        log()
        for state_i in xrange(len(self.states)):
            log('STATE: ',state_i)
            log()
            for ST_item in LR1_item_sets[state_i].items:
                log('  ', ST_item)
            log()
            for term_id, token_action in self.states[state_i].ACTION.iteritems():
                log('  on ',self.term_map[term_id],' ',token_action)
            log()
            for nonterm_id, goto_state in self.states[state_i].GOTO.iteritems():
                log('  goto ',goto_state,' on ',nonterm_map[nonterm_id])
            log()
            
#        if is_ambiguous:
#            return
                        
        ##compact action table
#        for current_state in self.states:
#            reduce_action=None
#            to_remove=[]
#            for token_id,token_action in current_state.ACTION.iteritems():
#                if token_action.is_reduce():
#                    to_remove.append(token_id)
#                    if reduce_action==None:
#                        reduce_action=token_action
#                    elif not reduce_action==token_action:
#                        print "ERROR: different reduces in same state:", current_state.
#            if reduce_action != None:
#                current_state.default_action=reduce_action
#            for rem in to_remove:
#                del current_state.ACTION[rem]
                
                        
        ##compact goto table (see pg 276-277) 
                        
        self.parser_info_generated=True
        
    def get_parser(self):
        
        if not self.parser_info_generated:
            self.log=logger()
            self.generate_parser_tables(self.log)
        
        new_lexer=self.lex_gen.get_lexer()
        new_parser=LRparser(new_lexer, self.term_map, self.production_info_list, self.error_function, self.terminal_error_recovery_table, self.states)
        
        return new_lexer,new_parser
        
class LRparser(object):
    def __init__(self, lexer, term_map, productions, error_function, terminal_recovery_table, states):
        self.lexer=lexer
        self.states=states
        self.term_map=term_map
        self.productions=productions
        
        self.error_function=error_function
        self.do_error_recovery=True
        self.terminal_recovery_table=terminal_recovery_table
        self.in_error_recovery_mode=False        
        
    ##for construction
    def set_error_recovery(self, do_error_recovery):
        self.do_error_recovery=do_error_recovery
        
#    def add_state(self, state):
#        self.states.append(state)

    ##running
        
    def state_str(self):
        out='[ '
        for state in self.stack:
            out+=str(state.id)+' '
        out+=']'
        return out
        
    def parse_step(self, reporting=False):
        if self.next_term==None: ##there was a lexer error
            return 2
        
        if reporting:
            print "NEXT TERMINAL:", self.term_map[self.next_term.id]
        
        state=self.states[ self.stack[-1].id ]
        act=state.action( self.next_term.id )
        if act.is_error():
            ##throw error
            expected='('
            for exp_token in state.ACTION.iterkeys():
                expected+=" "+self.term_map[exp_token]
            expected+=' )'
            self.error_function("parsing error- expected:"+expected+". Got a "+self.term_map[self.next_term.id], self.next_term.span)
            
            ##recover error
            total_error=True
            self.in_error_recovery_mode=True
            if self.do_error_recovery: ##do error recovery by poping off states and tokens.  Would like to change this so that popping tokens is prefered over states
                ##also would like to include possible alterations to tokens (see terminal.add_error_items)
                total_error=False
                recovering=True
                while recovering:
                    for stack_i in xrange(len(self.stack)-1, -1, -1):
                        looking_state_id=self.stack[ stack_i ].id
                        looking_state=self.states[ looking_state_id ]
                        act=looking_state.action( self.next_term.id )
                        if not act.is_error(): ##check the symbol itself
                            ##we found a symbol that doesn't error
                            self.stack=self.stack[:stack_i+1]
                            recovering=False
                        for rec_term_id in self.terminal_recovery_table[self.next_term.id]:
                            act=looking_state.action( rec_term_id )
                            if not act.is_error():
                                ##we found a symbol that doesn't error
                                self.stack=self.stack[:stack_i+1]
                                recovering=False
                                self.next_term.id=rec_term_id
                                break
                        if not recovering:
                            break
                                                            
                            
                    if recovering and self.term_map[self.next_term.id] == 'EOF':##make sure we are not in an endless loop
                        recovering=False
                        total_error=True
                    if recovering:
                        self.next_term=self.lexer()
                    
            ##report error
            if reporting:
                print "  ERROR"
                print "  STACK:",self.state_str()
            return 1 if total_error else 0
        elif act.is_reduce():
            if reporting:
                print "  REDUCING BY PRODUCTION:",act.argument
            production_info=self.productions[ act.argument ]
            ##pop the states off the stack
            datum=self.stack[-production_info.n_tokens:]
            self.stack=self.stack[:-production_info.n_tokens]
            ##run the user action
            new_data=None
            if production_info.action and not self.in_error_recovery_mode:
                new_data=production_info.action(datum)
            ##make the new state span
            new_span=location_span
            if len(datum)==1:
                new_span=datum[0].span
            elif len(datum)>1:
                new_span=datum[0].span+datum[-1].span
            ##make a new state
            new_state_id=self.states[ self.stack[-1].id ].goto(production_info.Lval_id)
            new_state=token_data( new_state_id,  new_data, new_span )
            self.stack.append(new_state)
            if reporting:
                print "  STACK:",self.state_str()
            return 0
        elif act.is_shift():
            if reporting:
                print "  SHIFTING TERMINAL"
            new_state_id=act.argument
            new_state=token_data( new_state_id,  self.next_term.data, self.next_term.span)
            self.stack.append(new_state)
            self.next_term=self.lexer()
            if reporting:
                print "  STACK:",self.state_str()
            return 0
        elif act.is_accept():
            if reporting:
                print "  PARSING COMPLETE"
                print "  STACK:",self.state_str()
            return 1
            
    def parse(self, reporting=False):
        self.stack=[ token_data(0,None,None) ]
        self.next_term=self.lexer()
        
        state=0
        while state==0:
            state=self.parse_step(reporting)
        if state==1 and not self.in_error_recovery_mode:
            return self.get_data()
            
    def get_data(self):
        return self.stack[-1].data
        
        
  #### for testing purposes
class parens_AST(object):
    
    @staticmethod
    def parse(tokens):
        return parens_AST(tokens[1].data)
    
    def __init__(self, AST_val):
        self.val=AST_val
        
    def str(self):
        return '('+self.val.str()+')'
        
class id_AST(object):
    
    @staticmethod
    def parse(tokens):
        return id_AST()
        
    def str(self):
        return 'id'
        
class product_AST(object):
    
    @staticmethod
    def parse(tokens):
        return product_AST(tokens[0].data, tokens[2].data)
        
    def __init__(self,LHS,RHS):
        self.LHS=LHS
        self.RHS=RHS
        
    def str(self):
        return '('+self.LHS.str()+'*'+self.RHS.str()+')'

class term_AST(object):
    
    @staticmethod
    def parse(tokens):
        return term_AST(tokens[0].data, tokens[2].data)
        
    def __init__(self,LHS,RHS):
        self.LHS=LHS
        self.RHS=RHS
        
    def str(self):
        return '('+self.LHS.str()+'+'+self.RHS.str()+')'
        
class neg_AST(object):
    
    @staticmethod
    def parse(tokens):
        return term_AST(tokens[0])
        
    def __init__(self,exp):
        self.exp=exp
        
    def str(self):
        return '(-'+self.exp.str()+')'
        
def pass_through(tokens):
    return tokens[0].data
      
if __name__=="__main__":
    lex_gen=lexer_generator()
    par_gen=parser_generator(lex_gen)
    
#    ##create terminals
#    ID=par_gen.new_terminal('id')
#    PLUS=par_gen.new_terminal('+')
#    MULT=par_gen.new_terminal('*')
#    L_PAREN=par_gen.new_terminal('(')
#    R_PAREN=par_gen.new_terminal(')')
#    
#    ##add potential error recovery tokens
#    L_PAREN.add_error_terms(R_PAREN)
#    R_PAREN.add_error_terms(L_PAREN)
#    
#    ##add lexer rules
#    lex_gen.add_pattern('id', ID )
#    lex_gen.add_pattern('+', PLUS )
#    lex_gen.add_pattern('*', MULT )
#    lex_gen.add_pattern('(', L_PAREN )
#    lex_gen.add_pattern(')', R_PAREN )
#    
#    ##non terminals
#    E=par_gen.new_nonterm('E')
#    T=par_gen.new_nonterm('T')
#    F=par_gen.new_nonterm('F')
#    
#    ##the grammer
#    E.add_production(E,PLUS,T, action=term_AST.parse )
#    E.add_production(T,        action=pass_through)
#    
#    T.add_production(T,MULT,F, action=product_AST.parse )
#    T.add_production(F,        action=pass_through)
#    
#    F.add_production(L_PAREN, E, R_PAREN, action=parens_AST.parse )
#    F.add_production(ID,       action=id_AST.parse )
    
#    ##grammer from example 4.61
#    ID=par_gen.new_terminal('id')
#    
#    ##lexer rools
#    lex_gen.add_pattern('id', ID)
#    
#    ##non terms
#    S=par_gen.new_nonterm('S')
#    L=par_gen.new_nonterm('L')
#    R=par_gen.new_nonterm('R')
#    
#    ##the grammer
#    S.add_production(L, '=', R)
#    S.add_production(R)
#    
#    L.add_production('*', R)
#    L.add_production(ID)
#    
#    R.add_production(L)
    
    ##grammer from section 4.8.1
    
    ##terminals
    ID=par_gen.new_terminal('id')
    
    ##lexer rules
    lex_gen.add_pattern(r'id', ID)
    lex_gen.add_pattern(r'" "|"\t"|"\n"')
    
    ## non terms
    E=par_gen.new_nonterm('E')
    
    ##grammer
    ADD_prod=E.add_production(E,'+',E,   action=term_AST.parse)
    MULT_prod=E.add_production(E,'*',E,  action=product_AST.parse)
    E.add_production('(',E,')',          action=parens_AST.parse)
    E.add_production(ID       ,          action=id_AST.parse)
    NEG_prod=E.add_production('-',E,     action=neg_AST.parse)
    
    ADD_prod.set_assoc('left')
    MULT_prod.set_assoc('left')
    
    par_gen.set_precedence( NEG_prod, MULT_prod, ADD_prod )
    
    lexer, parser=par_gen.get_parser()
    par_gen.log.write('./out.txt')

    lexer.unput( ' ( id+id+id )*id*id + id' )
    
    out=parser.parse(True)
    
    print
    if out!=None:
        print out.str()
    else:
        print "No Data"
    
    
    
    
    
    
    
        
        