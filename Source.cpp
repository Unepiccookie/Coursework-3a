#include <iostream>
#include <vector>

using namespace std;

enum state { Out_Of_Chocolate, No_Credit, Has_Credit, Dispenses_Chocolate, Maintenance_Mode };

class StateContext;

class State
{
protected:
	StateContext* CurrentContext;
public:
	State(StateContext* Context) { CurrentContext = Context; }
	virtual ~State(void) {}
};

class StateContext
{
protected:
	State* CurrentState = nullptr;
	int stateIndex = 0;
	vector<State*> availableStates;
public:
	virtual ~StateContext(void);
	virtual void setState(state newState);
	virtual int getStateIndex(void);
};

StateContext::~StateContext(void)
{
	for (int i = 0; i < this->availableStates.size(); i++) delete this->availableStates[i];
	this->availableStates.clear();
}

void StateContext::setState(state newState)
{
	this->CurrentState = availableStates[newState];
	this->stateIndex = newState;
}

int StateContext::getStateIndex(void)
{
	return this->stateIndex;
}

class Transition
{
public:
	virtual bool insertMoney(int) { cout << "Error!" << endl; return false; }
	virtual bool makeSelection(int) { cout << "Error!" << endl; return false; }
	virtual bool moneyRejected(void) { cout << "Error!" << endl; return false; }
	virtual bool addChocolate(int) { cout << "Error!" << endl; return false; }
	virtual bool dispense(void) { cout << "Error!" << endl; return false; }
	virtual bool enterPin(int pin) { cout << "Error!" << endl; return false; }
	virtual bool exit(void) { cout << "Error!" << endl; return false; }
};

class ChocoState : public State, public Transition
{
public:
	ChocoState(StateContext* Context) : State(Context) {}
};

class OutOfChocolate : public ChocoState
{
public:
	OutOfChocolate(StateContext* Context) : ChocoState(Context) {}
	bool enterPin(int pin);
	bool moneyRejected(void);
};

class NoCredit : public ChocoState
{
public:
	NoCredit(StateContext* Context) : ChocoState(Context) {}
	bool insertMoney(int credit);
	bool enterPin(int pin);
};

class HasCredit : public ChocoState
{
public:
	HasCredit(StateContext* Context) : ChocoState(Context) {}
	bool insertMoney(int credit);
	bool makeSelection(int option);
	bool moneyRejected(void);
};

class DispensesChocolate : public ChocoState
{
public:
	DispensesChocolate(StateContext* Context) : ChocoState(Context) {}
	bool dispense(void);
};

class MaintenanceMode : public ChocoState
{
public:
	MaintenanceMode(StateContext* Context) : ChocoState(Context) {}
	bool exit(void);
	bool addChocolate(int number);
};

class Chocolate_Dispenser : StateContext, public Transition
{
	friend class OutOfChocolate;
	friend class NoCredit;
	friend class HasCredit;
	friend class DispensesChocolate;
	friend class MaintenanceMode;
private:
	int inventory = 0; //how much chocolate the machine is currently holding
	int credit = 0; //How many bars can be purchased with the inserted money
	int pin = 54321; //Pin for maintenance - only change when compromised 
public:
	Chocolate_Dispenser(void);
	bool insertMoney(int credit);
	bool makeSelection(int option);
	bool moneyRejected(void);
	bool addChocolate(int number);
	bool dispense(void);
	bool enterPin(int pin);
	bool exit(void);

};

Chocolate_Dispenser::Chocolate_Dispenser(void)
{
	this->availableStates.push_back(new OutOfChocolate(this));
	this->availableStates.push_back(new NoCredit(this));
	this->availableStates.push_back(new HasCredit(this));
	this->availableStates.push_back(new DispensesChocolate(this));
	this->availableStates.push_back(new MaintenanceMode(this));

	this->setState(Out_Of_Chocolate);
}

bool Chocolate_Dispenser::insertMoney(int credit)
{
	return ((ChocoState*)CurrentState)->insertMoney(credit);
}
bool Chocolate_Dispenser::makeSelection(int option)
{
	return ((ChocoState*)CurrentState)->makeSelection(option);
}
bool Chocolate_Dispenser::moneyRejected(void)
{
	return ((ChocoState*)CurrentState)->moneyRejected();
}
bool Chocolate_Dispenser::addChocolate(int number)
{
	return ((ChocoState*)CurrentState)->addChocolate(number);
}
bool Chocolate_Dispenser::dispense(void)
{
	return ((ChocoState*)CurrentState)->dispense();
}
bool Chocolate_Dispenser::enterPin(int pin)
{
	return ((ChocoState*)CurrentState)->enterPin(pin);
}
bool Chocolate_Dispenser::exit(void)
{
	return ((ChocoState*)CurrentState)->exit();
}

bool MaintenanceMode::addChocolate(int number)
{
	((Chocolate_Dispenser*)CurrentContext)->inventory += number;
	cout << "Adding chocolate. Current inventory is: " << ((Chocolate_Dispenser*)CurrentContext)->inventory << endl;
	return true;
}
bool MaintenanceMode::exit(void)
{
	if (((Chocolate_Dispenser*)CurrentContext)->inventory == 0)
	{
		CurrentContext->setState(Out_Of_Chocolate);
		return true;
	}
	if (((Chocolate_Dispenser*)CurrentContext)->inventory > 0 && ((Chocolate_Dispenser*)CurrentContext)->credit == 0)
	{
		CurrentContext->setState(No_Credit);
		return true;
	}
	if (((Chocolate_Dispenser*)CurrentContext)->inventory > 0 && ((Chocolate_Dispenser*)CurrentContext)->credit > 0)
	{
		CurrentContext->setState(Has_Credit);
		return true;
	}
}

bool DispensesChocolate::dispense(void)
{
	((Chocolate_Dispenser*)CurrentContext)->inventory -= 1;
	((Chocolate_Dispenser*)CurrentContext)->credit -= 1;
	cout << "Chocolate dispensed. You have " << ((Chocolate_Dispenser*)CurrentContext)->credit << " credits left." << endl;
	if (((Chocolate_Dispenser*)CurrentContext)->inventory == 0)
	{
		CurrentContext->setState(Out_Of_Chocolate);
		return true;
	}
	if (((Chocolate_Dispenser*)CurrentContext)->inventory > 0 && ((Chocolate_Dispenser*)CurrentContext)->credit == 0)
	{
		CurrentContext->setState(No_Credit);
		return true;
	}
	if (((Chocolate_Dispenser*)CurrentContext)->inventory > 0 && ((Chocolate_Dispenser*)CurrentContext)->credit > 0)
	{
		CurrentContext->setState(Has_Credit);
		return true;
	}
}

bool HasCredit::insertMoney(int credit)
{
	((Chocolate_Dispenser*)CurrentContext)->credit += credit;
	cout << "You now have " << ((Chocolate_Dispenser*)CurrentContext)->credit << " credits." << endl;
}
bool HasCredit::makeSelection(int option)
{
	((Chocolate_Dispenser*)CurrentContext)->setState(Dispenses_Chocolate);
}
bool HasCredit::moneyRejected(void)
{
	cout << "Money has been rejected" << endl;
}

bool NoCredit::insertMoney(int credit)
{
	((Chocolate_Dispenser*)CurrentContext)->credit += credit;
	cout << "You now have " << ((Chocolate_Dispenser*)CurrentContext)->credit << " credits." << endl;
	((Chocolate_Dispenser*)CurrentContext)->setState(Has_Credit);
}
bool NoCredit::enterPin(int pin)
{
	if (pin == ((Chocolate_Dispenser*)CurrentContext)->pin)
	{
		cout << "Pin accepted" << endl;
		((Chocolate_Dispenser*)CurrentContext)->setState(Maintenance_Mode);
		return true;
	}
	else
	{
		cout << "Invaild pin" << endl;
	}
}

bool OutOfChocolate::moneyRejected(void)
{
	cout << "This machine is currently out of stock. Money will be returned" << endl;
}
bool OutOfChocolate::enterPin(int pin)
{
	if (pin == ((Chocolate_Dispenser*)CurrentContext)->pin)
	{
		cout << "Pin accepted" << endl;
		((Chocolate_Dispenser*)CurrentContext)->setState(Maintenance_Mode);
		return true;
	}
	else
	{
		cout << "Invalid pin" << endl;
	}
}

int main(void)
{

	while (true)
	{

	}
}