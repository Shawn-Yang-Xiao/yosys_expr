# Yosys


Yosys structs are analyzed and processed in our backend.


**RTLIL::Design** is defined in rtlil.h
components
```
	dict<RTLIL::IdString, RTLIL::Module*> modules_;
	std::vector<RTLIL::Binding*> bindings_;
```

`modules_` store hardware modules which build up the design.


**struct RTLIL::Module** is defined in rtlil.h
include components
```
	dict<RTLIL::IdString, RTLIL::Wire*> wires_;
	dict<RTLIL::IdString, RTLIL::Cell*> cells_;

	std::vector<RTLIL::SigSig>   connections_;
	std::vector<RTLIL::Binding*> bindings_;
```
and component `RTLIL::IdString name;` inherited from RTLIL::NamedObject


connect cell ports and wires, wires include input and output wires


**struct RTLIL::Wire** is defined in rtlil.h
include components
```
protected:
	RTLIL::Cell *driverCell_ = nullptr;
	RTLIL::IdString driverPort_;
public:
	RTLIL::Module *module;
	int width, start_offset, port_id;
	bool port_input, port_output, upto, is_signed;
```
and component `RTLIL::IdString name;` inherited from RTLIL::NamedObject


**struct RTLIL::Cell** is defined in rtlil.h
include components
```
	RTLIL::IdString type;
	dict<RTLIL::IdString, RTLIL::SigSpec> connections_;
	dict<RTLIL::IdString, RTLIL::Const> parameters;
```
and component `RTLIL::IdString name;` inherited from RTLIL::NamedObject


**struct RTLIL::Binding** is defined in binding.h



**RTLIL::SigSig** is defined by `typedef std::pair<SigSpec, SigSpec> SigSig;
` in rtlil.h (line 125). 


**struct RTLIL::SigSpec** is defined in rtlil.h
include components
```
private:
	int width_;
	Hasher::hash_t hash_;
	std::vector<RTLIL::SigChunk> chunks_; // LSB at index 0
	std::vector<RTLIL::SigBit> bits_; // LSB at index 0
```

output as std::string using `.as_string()`


**struct RTLIL::SigChunk** is defined in rtlil.h
```
	RTLIL::Wire *wire;
	std::vector<RTLIL::State> data; // only used if wire == NULL, LSB at index 0
	int width, offset;
```


**struct RTLIL::SigBit** is defined in rtlil.h,
with main components
```
	RTLIL::Wire *wire;
	union {
		RTLIL::State data; // used if wire == NULL
		int offset;        // used if wire != NULL
	};
```


**RTLIL::State** is defined in rtlil.h, line 33 as
```
	enum State : unsigned char {
		S0 = 0,
		S1 = 1,
		Sx = 2, // undefined value or conflict
		Sz = 3, // high-impedance / not-connected
		Sa = 4, // don't care (used only in cases)
		Sm = 5  // marker (used internally by some passes)
	};
```

**struct RTLIL::Const** is defined in rtlil.h
```
private:
	union {
		bitvectype bits_;
		std::string str_;
	};
	bitvectype* get_if_bits() { return is_bits() ? &bits_ : NULL; }
	std::string* get_if_str() { return is_str() ? &str_ : NULL; }
	const bitvectype* get_if_bits() const { return is_bits() ? &bits_ : NULL; }
	const std::string* get_if_str() const { return is_str() ? &str_ : NULL; }

```




