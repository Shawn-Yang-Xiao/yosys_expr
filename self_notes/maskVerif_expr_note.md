
Go through mv file read process to get necessary OCaml types, in order to generate mv program thourgh expression we will define.

In main.ml 
`let rec process_command c = ... | Func f -> pp_added (Prog.Process.func globals f)`

-> In prog.ml, in module Process
```OCaml
  let func globals f =
    let func = ToProg.to_func globals f in
    let func = macro_expand_func globals func in
    add_global globals func;
    func
```

-> In prog.ml, in module ToProg
```OCaml
  let to_func globals func =
    let f_name = HS.make (data func.P.f_name) in
    let env, f_pin, f_in, f_out, f_rand = init_shared globals func in
    let f_cmd = List.flatten (List.map (to_instr env) func.P.f_cmd) in
    let f_other = env.others in
    { f_name; f_pin; f_in; f_out; f_other; f_rand; f_cmd }
```

Now focus on how f_cmd is generated.

-> In prog.ml, in module ToProg
```OCaml
  let to_instr env i =
    match (data i) with
    | P.Iassgn id -> to_assgn env (mkloc (loc i) id)
    | P.Imacro id -> to_macro env (mkloc (loc i) id)
    | P.Ileak (id,msg) -> to_leak env (mkloc (loc i) (id,msg))
```

We first focus on assign statement.
-> In 


Use `get_vcall` to read lhs




From another perspective, look into data type produced by `ToProg.to_func`.
That is 
```Ocaml
type func = {
  f_name   : ident;
  f_pin    : (ident * range option * Expr.ty) list;
  f_in     : (ident * ids * Expr.ty) list;
  f_out    : (ident * ids * Expr.ty) list;
  f_shares : (ident * ids * Expr.ty) list;
  f_rand   : (ident * range option * Expr.ty) list;
  f_other  : (ident * Expr.ty) list;
  f_cmd    : cmd }
```

We focus on `cmd` type, in prog.ml : `type cmd = instr list`

Further, 
```OCaml
type instr_d =
  | Ileak of leak
  | Iassgn of assgn
  | Imacro of macro_call
```

`type assgn = {i_var : var; i_kind : P.instr_kind; i_expr : expr }`, left hand side `type var = E.var` is 
```OCaml
type var = {
  v_id   : int;
  v_name : string;
  v_ty   : ty;
}
```
in expr.ml.

Take a look at Parsetree.instr_kind, how many instruction kind do we need?
In maskVerif, there are:
```OCaml
type instr_kind =
  | IK_subst (* := *)
  | IK_hide (* = [ ] *)
  | IK_sub (* = *)
  | IK_glitch (* =![ ] *)
  | IK_noleak (* <- *)
```
Among them, `IK_subst` is used at wire assignment, `IK_glitch` is used at reg assignment.




Next, we see how the data types are processed.
Now `func` is added into globals (skip `macro_expand_func` for now).

Take NI check as an example, when it process into `let build_obs_func ~ni ~trans ~glitch loc f ` . Then further process in `  build_obs ~trans ~glitch obs s f.f_cmd`  .
Now comes to `let rec build_obs ~trans ~glitch obs s c` . 






# In expr.ml file

### `ty` 

**Meaning:** includes types of 1/8/16/32/64 bits, but we only need 1 bit bool type in cell definitions.

Meanwhile, top module still requires multiple bit signal, so we should decide how to deal with expression like `a[1] := b[1] & c[1]` in mv language.

**Usage:** 


### `var` 

**Meaning**: with id, name and type.

**Usage**: 


###  



