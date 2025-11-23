# genrtlil

generate rtlil process is used to collect rtlil IR cells for translation.


List of cells
```
$not, $pos, $neg, $and, $or, $xor, $xnor, 
$reduce_and, $reduce_or, $reduce_xor, $reduce_xnor, $reduce_bool, 
$shl, $shr, $sshl, $sshr, 
$shiftx, $shift,
$lt, $le, $eq, $ne, $eqx, $nex, $ge, $gt,
$add, $mul, $sub, $div, $mod, $mux
$logic_and, $logic_or, $logic_not, 
$memrd, $meminit_v, $check, $specify2, $specify3, $specrule, $print, 
```


Implementation is stored in techlibs/common/simlib.v



```
if (cell->type == ID($not)) return export_bvop(cell, "(bvnot A)");
```

```
if (cell->type.in(ID($buf), ID($pos), ID($_BUF_))) {
	return a;
} else if (cell->type.in(ID($not), ID($_NOT_))) {
	return NOT(a);
```



