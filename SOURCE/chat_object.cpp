	/***********************************************************/
	/*           STL Based Chat Object Implementation          */
	/***********************************************************/

	#include <list>
	#include <unordered_map>
	#include "chat_object.h"
	#include "sha1.h"

	using namespace std;

	#define DEBUG	0

	#if DEBUG
		#include <iostream>
	#endif

	/***********************************************************/
	/*                        List Class                       */
	/***********************************************************/

	class List_Object {
	public:
		virtual unsigned size() { return lst.size(); };

		virtual ID init_Element()
		{
			if (lst.size() == 0)return NOTANID;
			return *lst.begin();
		};

		virtual ID last_Element()
		{
			if (lst.size() == 0)return NOTANID;
			it = lst.end();
			return *(--it);
		};

		virtual ID next_Element()
		{
			if (it == lst.end())return NOTANID;
			if (++it == lst.end())return NOTANID;
			return *it;
		};

		virtual ID prev_Element()
		{
			if (it == lst.begin())return NOTANID;
			return *(--it);
		};

		virtual void include_Element(size_t x) { lst.push_front(x); };

	private:
		list<ID> lst;
		list<ID>::iterator it = lst.end();
	};

	/***********************************************************/

	class List_Class : public List_Access {
	public:

		virtual void open(ID id) { lptr = (List_Object*)id; };

		virtual ID create() override	// create list object
		{
			lptr = new List_Object();
			ID id = ID(lptr);
#if DEBUG
			cout << " list created id = " << id << endl;
			cout << endl;
#endif		lptr = new List_Object();
			return id;
		};

		virtual unsigned Size() override { return lptr->size(); }

		virtual ID init_Element() override { return lptr->init_Element(); };
/*
			init list element, returns NOTANID if the list is empty
*/
		virtual ID last_Element() override { return lptr->last_Element(); };
/*
			last list element, returns NOTANID if the list is empty
*/
		virtual ID next_Element() override { return lptr->next_Element(); };
/*
			next list element, can return NOTANID
*/
		virtual ID prev_Element() override { return lptr->prev_Element(); };
/*
			previous list element can return NOTANID
*/
		virtual void include_Element(ID id) override { lptr->include_Element(id); };
/*
			include element to the beginning
*/
		List_Class():List_Access() {};

	private:
		List_Object* lptr = nullptr;
	};	

	/***********************************************************/
	/*                         User Class                      */
	/***********************************************************/

	class List_Object;

	class User_Object {
	public:
		string name;			
		List_Object* group_list = nullptr;		// group list 
		List_Object* message_list = nullptr;	// message list object id
		Digest digest;							// password digest 

		User_Object(const char* _name, const char* _pass) 
		{
			name = _name;
			sha1(_pass, strlen(_pass), digest);
			group_list = new List_Object();
			message_list = new List_Object();
		};

		// alternative constructor for wrapping

		User_Object(const char* _name) { name = _name; };

		~User_Object() {};
	};

	/***********************************************************/

	class User_Class : public User_Access {
	public:

		virtual void open(ID id);

		virtual ID create(const char* name, const char* pass) override;

		virtual const char* allocate_Name() const override;			// user name
		virtual bool valid_Password(const char* pass) const;		// check password
		virtual ID message_Link_List() const override;				// message link list ID
		virtual ID group_List() const override;						// group list

		virtual void change_Password(const char* pass) override;	// change password

		User_Class():User_Access(){};

	private:
		User_Object* user = nullptr;
	};

	/***********************************************************/

	const char* allocate_Line(const char* s) 
	{
		unsigned size = strlen(s) + 1;
		char* sptr = (char*)malloc(size);
		if(sptr != nullptr)memcpy(sptr, s, size);
		return sptr;
	};

	void User_Class::open(ID id) { user = (User_Object*)id; };

	/***********************************************************/

	const char* User_Class::allocate_Name()const 
	{
		return allocate_Line((user->name).c_str());
	};

	/***********************************************************/

	bool User_Class::valid_Password(const char* pass) const
	{
		Digest digest;
		sha1(pass, strlen(pass), digest);
		for (unsigned i = 0; i < 5; i++)
			if (user->digest[i] != digest[i])return false;
		return true;
	};

	ID User_Class::group_List() const { return (ID)(user->group_list);	};

	ID User_Class::message_Link_List() const { return (ID)(user->message_list); };

	/***********************************************************/

	void User_Class::change_Password(const char* pass) 
	{
		sha1(pass, strlen(pass), user->digest);
	};

	/***********************************************************/

	ID User_Class::create(const char* name, const char* pass)
	{
		user = new User_Object(name, pass);
		ID user_id = (ID)user;
#if DEBUG
		cout << " user created id = " << user_id << endl;
		cout << " name = " << name << endl;
		cout << " pass = " << pass << endl;
		cout << endl;
#endif
		return user_id;
	};

	/***********************************************************/
	/*                     ID Table Class                      */
	/***********************************************************/

	const char* allocate_Key(size_t n) 
	{
		User_Object* user = (User_Object*)n;
		return allocate_Line(user->name.c_str());
	};

	/***********************************************************/
	/*                    User List Class                      */
	/***********************************************************/

	#include <unordered_set>

	class User_Ptr {		// supplementary class representing table elements
	public:

		const User_Object* get_User() { return user; };

		string key() const { return user->name; };

		User_Ptr(const User_Object* u) { user = u; };

	private:
		const User_Object* user;
	};

	bool operator==(const User_Ptr& lhs, const User_Ptr& rhs)	// compare for equality
	{
		return lhs.key() == rhs.key();
	};

	struct Hash_User_Ptr										// key based hash calculation
	{
		size_t operator()(const User_Ptr& x) const
		{
			return std::hash<string>{}(x.key());				// std::hash<string>
		};
	};

	using user_list = unordered_set<User_Ptr, Hash_User_Ptr, equal_to<>>;

	/***********************************************************/

	class User_List : public User_List_Access
	{
	public:

		virtual void open(ID id)override {};

		virtual ID create() { return 0; };

		virtual ID find_User(const char* name) override 
		{
			User_Object user(name);                     // wrapping name in User
			it = table.find(User_Ptr(&user));			// wrapping pointer in User_Ptr
			if (it == table.end())return NOTANID;
			User_Ptr uptr = *it;
			return (ID)uptr.get_User();					// convert into ID
		};

		virtual ID init_User(unsigned* nptr) override 
		{
			if (table.size() == 0)return NOTANID;
			it = table.begin();
			User_Ptr uptr = *it;
			return (ID)uptr.get_User();					// convert into ID
		};

		virtual ID next_User(unsigned* nptr) override 
		{
			if (it == table.end())return NOTANID;
			if (++it == table.end())return NOTANID;
			User_Ptr uptr = *it;
			return (ID)uptr.get_User();					// convert into ID
		};

		virtual void new_User(ID user_id, const char* name) override 
		{
			User_Object* user = (User_Object*)user_id;	// convert to User_Object pointer 
			table.insert(User_Ptr(user));				// wrapping pointer in User_Ptr
		};
		
		User_List() {};
		
		virtual ~User_List() {};

	private:
		user_list table;
		user_list::iterator it = table.end();
	};

	/***********************************************************/
	/*                         Message Class                   */
	/***********************************************************/

	class Group_Object;

	class Message_Object {
	public:
		string text;							// text block id
		time_t time = 0;						// timestamp

		User_Object* sender = nullptr;
		User_Object* recipient = nullptr;
		Group_Object* group = nullptr;

		Message_Object(const char* _text,
			User_Object* _sender,
			User_Object* _recipient,
			Group_Object* _group)
		{
			text = _text;
			sender = _sender;
			recipient = _recipient;
			group = _group;
		};
	};

	/***********************************************************/

	class Message_Class : public Message_Access {
	public:
		virtual void open(ID id)override;

		virtual ID create(const char* text, 
			ID sender_id, 
			ID recipient_id,
			ID group_id) override;	// create a message


		virtual const char* allocate_Text() const override;		// message text
		virtual time_t message_Time() const override;			// message timestamp
		virtual ID sender_ID() const override;					// message sender ID
		virtual ID recipient_ID()const override;				// message recipient ID
		virtual ID group_ID() const override;					// message group id
	
		Message_Class() : Message_Access() {};

	private:
		Message_Object* message = nullptr;
	};

	/***********************************************************/

	const char* Message_Class::allocate_Text() const 
	{
		return allocate_Line(message->text.c_str());
	};

	time_t Message_Class::message_Time() const { return message->time; };

	ID Message_Class::sender_ID() const { return (ID)(message->sender); };

	ID Message_Class::recipient_ID()const { return (ID)(message->recipient); };

	ID Message_Class::group_ID()const { return ID(message->group); };

	/***********************************************************/

	void Message_Class::open(ID id) { message = (Message_Object*)id; };

	ID Message_Class::create(const char* text, ID sender_id, ID recipient_id, ID group_id)
	{
		message = new Message_Object(text,
			(User_Object*)sender_id, (User_Object*)recipient_id, (Group_Object*)group_id);
		ID id = (ID)message;
#if DEBUG
		cout << " message created id = " << id << endl;
		cout << " text = " << text << endl;
		cout << endl;
#endif
		return id;
	};

	/***********************************************************/
	/*                    Message Link Class                   */
	/***********************************************************/

	class Message_Link_Object {
	public:
		bool sent = false;					// sent
		bool received = false;				// received
		Message_Object* message = nullptr;	// message object id

		Message_Link_Object(bool _sent, bool _received, Message_Object* _message) 
		{
			sent = _sent;
			received = _received;
			message = _message;
		};
	};

	/***********************************************************/

	class Message_Link_Class : public Message_Link_Access {
	public:

		virtual void open(ID id)override;
		virtual ID create(bool sent, bool received, ID message_id) override;	// create a link

		virtual bool message_Sent() const override;				// message is sent
		virtual bool message_Received() const override;			// message is received
		virtual ID message_ID() const override;					// message ID

		Message_Link_Class() :Message_Link_Access() {};

	private:
		Message_Link_Object* message_link = nullptr;
	};

	/***********************************************************/

	void Message_Link_Class::open(ID id) { message_link = (Message_Link_Object*)id; };

	ID Message_Link_Class::create(bool sent, bool received, ID message_id) 
	{
		message_link = new Message_Link_Object(sent, received, (Message_Object*)message_id);
		ID id = (ID)message_link;
#if DEBUG
		cout << " message link created id = " << id << endl;
		cout << endl;
#endif
		return id;
	};

	/***********************************************************/

	bool Message_Link_Class::message_Sent() const { return message_link->sent; };

	bool Message_Link_Class::message_Received() const { return message_link->received; };

	ID Message_Link_Class::message_ID() const { return (ID)(message_link->message); };

	/***********************************************************/
/*                        Group Class                      */
/***********************************************************/

	class Group_Object {
	public:
		string name;					// group name
		List_Object* user_list;			// user list object

		Group_Object(const char* _name) 
		{
			name = _name;
			user_list = new List_Object();
		};
	};

	/***********************************************************/

	class Group_Class : public Group_Access {
	public:
		virtual void open(ID id);

		virtual ID create(const char* name) override;			// create gruop

		virtual const char* allocate_Name() const override;		// group name
		virtual ID user_List_ID() const override;				// group user list

		Group_Class() :Group_Access() {};

	private:
		Group_Object* group = nullptr;
	};

	/***********************************************************/

	void Group_Class::open(ID id) { group = (Group_Object*)id; };

	const char* Group_Class::allocate_Name() const
	{
		return allocate_Line(group->name.c_str());
	};

	/***********************************************************/

	ID Group_Class::user_List_ID() const
	{
		return (ID)(group->user_list);
	};

	/***********************************************************/

	ID Group_Class::create(const char* name)
	{

		group = new Group_Object(name);
		ID id = (ID)group;
#if DEBUG
		cout << " group created id = " << id << endl;
		cout << " name = " << name << endl;
		cout << endl;
#endif
		return id;
	};

	/***********************************************************/
	/*                        Chat Class                       */
	/***********************************************************/

	class Chat_Class : public Chat_Class_Access
	{
	public:

		virtual void open(ID id)override {};

		virtual User_Access* create_User_Access() override
		{ 
			return new User_Class(); 
		};

		virtual Message_Access* create_Message_Access() override
		{ 
			return new Message_Class(); 
		};

		virtual Message_Link_Access* create_Message_Link_Access() override
		{ 
			return new Message_Link_Class(); 
		};

		virtual Group_Access* create_Group_Access() override 
		{
			return new Group_Class();
		};
		
		virtual List_Access* create_List_Access() override
		{
			return new List_Class();
		};

		virtual User_List_Access* create_User_List_Access() override
		{
			return new User_List();
		};

		virtual ID user_List_ID() override { return (ID)user_list; };

		virtual void close_Chat_Class() override {};

	private:
		User_Class* user = nullptr;
		User_List* user_list = nullptr;
	};

	/***********************************************************/

	static Chat_Class chat_class;	// static object

	Chat_Class_Access* get_Chat_Class() 
	{
		return &chat_class;							// return chat_class
	};

	/***********************************************************/

