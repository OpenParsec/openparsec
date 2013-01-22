#ifndef E_SIMNETINPUT_H_
#define E_SIMNETINPUT_H_

// forward decls --------------------------------------------------------------
//
class E_REList;
//struct RE_Header;
//struct ShipRemInfo;


// class for handling the network input from the simulation -------------------
//
class E_SimNetInput {
protected:
	E_REList*	m_pInputREList;

protected:
	// default ctor/dtor
	E_SimNetInput();
	~E_SimNetInput();

public:
	// SINGLETON access
	static E_SimNetInput* GetSimNetInput()
	{
		static E_SimNetInput _TheSimNetInput;
		return &_TheSimNetInput;
	}

	// reset all data
	void Reset();

	// get the input remote event list
	//E_REList* GetInputREList() { return m_pInputREList; }

	// walk the input RE queue and apply events/states to simulation
	void ProcessInputREList();

	// handle the network input from a client
	int HandleOneClient( int nClientID, RE_Header* relist );

protected:
	// check absolute movement bounds
	int _CheckAbsoluteMovementBounds( RE_PlayerAndShipStatus* pPAS );
};

#endif // !E_SIMNETINPUT_H_

