namespace stx {

namespace detail {

// == Result policies ========================================================

namespace result_policy {

struct ignore_struct {};
struct collect_struct {};
struct first_true_struct {};
struct first_false_struct {};

template<typename T>
struct default_policy_struct;

} // namespace result_policy


template<typename Callback, typename Result, typename... ARGS>
class default_observer_implementation;

// == Observer list ==========================================================

struct handle_socket {
	virtual void on_handle_destroyed() noexcept = 0;
};

template<typename Result, typename... ARGS>
class observer_interface : public handle_socket {
	using Tself = observer_interface<Result, ARGS...>;

	Tself** m_head;
	Tself*  m_next;
	Tself*  m_last;

public:
	template<typename Callback>
	using default_implementation = default_observer_implementation<Callback, Result, ARGS...>;

	observer_interface() :
		m_head(nullptr),
		m_next(nullptr),
		m_last(nullptr)
	{}

	// -- Move --------------------------------------------------------------
	observer_interface(Tself&& other) :
		observer_interface()
	{
		*this = std::move(other);
	}

	observer_interface& operator=(Tself&& other) {
		detach();
		m_head = other.m_head;
		m_next = other.m_next;
		m_last = other.m_last;

		if(m_next) {
			m_next->m_last = this;
		}

		if(m_last) {
			m_last->m_next = this;
		}
		else if(m_head) {
			*m_head = this;
		}

		other.m_head = nullptr;
		other.m_next = nullptr;
		other.m_last = nullptr;

		return *this;
	}

	observer_interface(Tself const&) = delete;
	observer_interface& operator=(Tself const&) = delete;

	void detach() {
		if(m_head) {
			if(m_next) {
				m_next->m_last = m_last;
			}

			if(m_last) {
				m_last->m_next = m_next;
			}
			else {
				*m_head = m_next;
			}

			m_head = nullptr;
			m_next = nullptr;
			m_last = nullptr;

			on_detach();
		}
	}

	void attach_to(Tself** head) {
		detach();

		m_head = head;

		if(*m_head == nullptr) {
			*m_head = this;
			m_next  = m_last = nullptr;
		}
		else {
			m_next         = *m_head;
			m_next->m_last = this;
			*m_head        = this;
		}
	}

	Tself* next() { return m_next; }

	void on_handle_destroyed() noexcept final override { detach(); }

	virtual ~observer_interface() { detach(); }
	virtual Result on_event(ARGS... args) = 0;
	virtual void   on_detach() = 0;
};

template<typename Callback, typename Result, typename... ARGS>
class default_observer_implementation : public observer_interface<Result, ARGS...> {
	Callback callback;

public:
	template<typename CallbackX>
	default_observer_implementation(CallbackX&& c) :
		callback(std::forward<CallbackX>(c))
	{}

	Result on_event(ARGS... args) final override {
		return callback(std::forward<ARGS>(args)...);
	}

	void on_detach() final override {
		delete this;
	}
};

template<typename FnSig>
struct observer_interface_of_struct;

template<typename Result, typename... ARGS>
struct observer_interface_of_struct<Result(ARGS...)> {
	using type = observer_interface<Result, ARGS...>;

	template<typename Callback>
	using default_implementation = typename observer_interface<Result, ARGS...>::template default_implementation<Callback>;
};

template<typename FnSig>
using observer_interface_of = typename observer_interface_of_struct<FnSig>::type;

template<typename FnSig, typename Callback>
observer_interface_of<FnSig>* create_new_observer(Callback&& fn) {
	using CallbackType = std::remove_const_t<std::remove_reference_t<Callback>>;
	using observer = observer_interface_of<FnSig>;
	using observer_impl = typename observer:: template default_implementation<CallbackType>;

	return new observer_impl(std::forward<Callback>(fn));
}

} // namespace detail

} // namespace stx
