#include <iostream>
#include <map>
#include <stdexcept>
#include <functional>

using namespace std;

template <class T>
function<bool (const T& value)> checkMinMax(T min, T max)
{
	return [min, max] (const T& value) -> bool {return (value >= min && value <= max);};
}

function<bool(const string& s)> checkMaxLength(size_t maxLen)
{
	return [maxLen] (const string& s) -> bool {return s.length() <= maxLen;};
}

class Variant
{
	public:
		enum type
		{
			INT,
			DOUBLE,
			STRING
		};

		Variant(int i): _type(Variant::INT), _i(i) {}
		Variant(double d): _type(Variant::DOUBLE), _d(d) {}
		Variant(const string& s): _type(Variant::STRING), _s(s) {}
		Variant(const char* s): _type(Variant::STRING), _s(s) {}

		Variant& operator=(int i)
		{
			_type = Variant::INT;
			_i = i;
			return *this;
		}

		Variant& operator=(double d)
		{
			_type = Variant::DOUBLE;
			_d = d;
			return *this;
		}

		Variant& operator=(const string& s)
		{
			_type = Variant::STRING;
			_s = s;
			return *this;
		}

		operator int() const
		{
			if (_type != Variant::INT)
				throw runtime_error("Not an int");
			return _i;
		}
		operator double() const {
			if (_type != Variant::DOUBLE)
				throw runtime_error("Not a double");
			return _d;
		}

		operator string() const {
			if (_type != Variant::STRING)
				throw runtime_error("Not a string");
			return _s;
		}

	private:
		type _type;
		union
		{
			int _i;
			double _d;
		};
		std::string _s;

};

class Object;

class GenericParam
{
	public:
		GenericParam(const std::string& name): _name(name) {}

		virtual Variant getAsVariant() const = 0;
		virtual void setFromVariant(const Variant& v) = 0;

		const string& getName() const {return _name;}

	private:
		string _name;
};

template <class T>
class Param: public GenericParam
{
	public:
		typedef function<bool(const T&)> Validator;

		Param(Object* parent, const string& name, const T& initialValue, const T& min, const T& max);
		Param(Object* parent, const string& name, const T& initialValue, const Validator& validator);

		virtual Variant getAsVariant() const {return Variant(*this);}
		virtual void setFromVariant(const Variant& v) {*this = v;}

		T getValue() const {return _value;}

		T& operator=(const T& other)
		{
			if (!_validator(other))
				throw range_error("Value not valid!");

			return _value = other;
		}

		operator T() const
		{
			return _value;
		}


	private:
		string		_name;
		T 			_value;
		const function<bool(const T&)> _validator;
};

class Object
{
	public:
		Object():
			i(this, "myInt", 0, -10, 10),
			d(this, "myDouble", 0.0, -10.0, 10),
			s(this, "myString", "string", [](const string& v) -> bool {return v.length() <= 20;})
		{}

		void registerParam(GenericParam* p)
		{
			auto pi = params.find(p->getName());
			if (pi != params.end())
				throw runtime_error("Name already used!");

			params[p->getName()] = p;
		}

		void listParams() const
		{
			for (auto p: params)
			{
				cout << p.first << endl;
			}
		}

		void setParam(const string& name, Variant value)
		{
			auto pi = params.find(name);
			if (pi == params.end())
				throw runtime_error("Param not found!");

			(*pi).second->setFromVariant(value);
		}

		Variant getParam(const string& name)
		{
			auto pi = params.find(name);
			if (pi == params.end())
				throw runtime_error("Param not found!");

			return (*pi).second->getAsVariant();
		}

		void zero()
		{
			i = 0;
			d = 0.0;
			s = "zero";
		}

	private:
		map<string, GenericParam*> params;

		Param<int> i;
		Param<double> d;
		Param<string> s;
};

template <class T>
Param<T>::Param(Object* parent, const string& name, const T& initialValue, const Validator& validator):
	GenericParam(name),
	_value(initialValue), _validator(validator)
{
	if (parent)
		parent->registerParam(this);
}

template <class T>
Param<T>::Param(Object* parent, const string& name, const T& initialValue, const T& min, const T& max):
	Param(parent, name, initialValue, checkMinMax(min, max))
{
}



int main()
{
	Object o;

	cout <<  "Registered params:" << endl;

	o.listParams();

	cout << "Setting myInt = 5:" << endl;
	o.setParam("myInt", 5);
	cout << (int)o.getParam("myInt") << endl;

	try
	{
		o.setParam("myString", "012345678901234567890");
	}
	catch (const std::exception& e)
	{
		cout << "Setting myString failed - string too long" << endl;
		cout << e.what() << endl;
	}

  try
	{
		o.setParam("myDouble", "123");
	}
	catch (const std::exception& e)
	{
		cout << "Setting myDouble to string failed" << endl;
		cout << e.what() << endl;
	}

	cout << "Zeroing all the fields from the class itself" << endl;

	o.zero();
	cout << "myInt:    " << (int)o.getParam("myInt") << endl;
	cout << "myString: " << (string)o.getParam("myString") << endl;
	cout << "myDouble: " << (double)o.getParam("myDouble") << endl;

	return 0;
}
