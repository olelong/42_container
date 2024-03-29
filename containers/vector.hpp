#ifndef VECTOR_HPP
# define VECTOR_HPP

# include <iostream>
# include <string>
# include <sstream>
# include <stdexcept>
# include <typeinfo>
# include "../iterators/vector_iterator.hpp"
# include "../iterators/reverse_iterator.hpp"
# include "../utils/iterator_traits.hpp"
# include "../utils/lexicographical_compare.hpp"
# include "../utils/enable_if.hpp"
# include "../utils/is_integral.hpp"

namespace ft
{
	template<class T, class Alloc = std::allocator<T> >
	class vector
	{
		public:
			typedef 			T												value_type;
			typedef 			Alloc											allocator_type;
			typedef	typename	allocator_type::reference						reference;			
			typedef typename	allocator_type::const_reference					const_reference;			
			typedef	typename	allocator_type::pointer							pointer;			
			typedef typename	allocator_type::const_pointer					const_pointer;			
			typedef 			vector_iterator<value_type>						iterator;
			typedef 			vector_iterator<const value_type>				const_iterator;
			typedef 			ft::reverse_iterator<iterator>					reverse_iterator;
			typedef				ft::reverse_iterator<const_iterator>			const_reverse_iterator;
			typedef typename	ft::iterator_traits<iterator>::difference_type	difference_type;
			typedef 			size_t											size_type;
			

			// Constructors & Destructor
			explicit vector(const allocator_type& alloc = allocator_type()) 		// empty container
				: arr(NULL), _alloc(alloc), _size(0), _capacity(0), old_resize(0){}

			explicit vector(size_type n, const value_type& val = value_type(),   	// fill container
				const allocator_type& alloc = allocator_type()) 
				: arr(NULL), _alloc(alloc), _size(0), _capacity(0), old_resize(0){
					this->assign(n, val);
			}

			template <class InputIterator>
			vector (InputIterator first, InputIterator last,						// range container
				const allocator_type& alloc = allocator_type(),
				typename ft::enable_if<!ft::is_integral<InputIterator>::value, InputIterator>::type* = NULL)
				: arr(NULL), _alloc(alloc), _size(0), _capacity(0), old_resize(0) {
					this->assign(first, last);
				}
			
			vector (const vector &x) : arr(NULL), _size(0), _capacity(0), old_resize(0) {	// copy constructor
				*this = x;
			}
						
			~vector() {
				if (this->arr != NULL)
				{
					for (size_type i = 0; i < this->_size; i++)
						this->_alloc.destroy(&this->arr[i]);
					this->_alloc.deallocate(this->arr, this->_capacity);
				}
			}
			
			// Operators=
			vector& operator=(const vector& x) {
				if (this != &x)
					this->assign(x.begin(), x.end());
				return *this;
			}

			// Iterators

										/*  Iterator  */
			iterator begin() {
				return iterator(this->arr);
			}
			
			const_iterator begin() const {
				return const_iterator(this->arr);
			}

			iterator end() {
				return iterator(arr + this->_size);
			}

			const_iterator end() const {
				return const_iterator(this->arr + this->_size);
			}
									/* Reverse Iterator  */
			reverse_iterator rbegin() {
				return reverse_iterator(this->end());
			}

			const_reverse_iterator rbegin() const {
				return const_reverse_iterator(this->end());
			}

			reverse_iterator rend() {
				return reverse_iterator(this->begin());
			}

			const_reverse_iterator rend() const {
				return const_reverse_iterator(this->begin());
			}

			// Capacity
			size_type size() const {   // Return the number of elements in the vector
				return this->_size;
			}

			size_type max_size() const {	// Return the maximum number of elements that the vector can hold
				return this->_alloc.max_size();
			}

			void resize(size_type n, value_type val = value_type()) { // Resizes the container, it contains n elements
				if (n < this->_size)
					for (size_t i = n; i < this->_size; i++)
						this->_alloc.destroy(this->arr + i);
				else {
					if (n > this->_capacity) {
						size_type new_capacity;
						if (n > this->old_resize == 0 || n > this->old_resize * 2)
							new_capacity = n;
						else
							new_capacity = this->old_resize * 2; // add a capacity de size * 2 of the previous size
						this->reallocate(new_capacity);
					}
					for (size_type i = this->_size; i < n; i++)
						this->_alloc.construct(this->arr + i, val);
				}
				this->_size = n;
				this->old_resize = n;
			}

			size_type capacity() const {	// Return the size of the storage space currently allocated for the vector
				return this->_capacity;
			}

			bool empty() const {	// Test if the vector is empty
				if (_size == 0)
					return true;
				return false;
			}

			void reserve(size_type n) { // Request a change in capacity, for the container be at leat enough to contain n elements
				if (n > this->max_size())
					throw(std::length_error("vector::reserve"));
				if (this->_capacity >= n)
					return ;
				size_type	new_capacity = n;
				this->reallocate(new_capacity);
			}
			
			// Element Access
			reference operator[] (size_type n) {
				return this->arr[n]; 
			}
			
			const_reference operator[] (size_type n) const{
				return this->arr[n]; 
			}

			reference at(size_type n) {		// Return a reference to the element at position n in the vector
				if (n >= this->_size)	// It checks whether n is within the bounds of valid elements in contrast with operator[]
					throw std::out_of_range(this->out_of_range_what(n));
				return this->arr[n];
			}

			const_reference at(size_type n) const {
				if (n >= this->_size)
					throw std::out_of_range(this->out_of_range_what(n));
				return this->arr[n];
			}

			reference front() {		// Access first element in the vector
				return this->arr[0];
			}
			
			const_reference front() const {
				return this->arr[0];
			}
			
			reference back() {		// Access last element in the vector
				return this->arr[_size - 1];
			}
			
			const_reference back() const {
				return this->arr[_size - 1];
			}

			value_type* data() {	// Returns a direct pointer to the memory array used by the vector to store its owned elements
				return this->arr;
			}

			const value_type* data() const {
				return this->arr;
			}

			// Modifiers

								/*   Assign    */
			// Assign new contents to the vector, replacing its current contents and modifying its size accordingly
			template <class InputIterator> // Range // New contents constructed in the range between first and last
				void assign(InputIterator first, InputIterator last, 
					typename ft::enable_if<!ft::is_integral<InputIterator>::value, InputIterator>::type* = NULL)  {
					size_type new_size = 0;
					size_type i = 0;
					InputIterator it = first;

					while (it != last) { // Recover the size of new contents to add
						new_size++;
						it++;
					}
					reserve(new_size);

					if (new_size < this->_size) { // Check if allocation is needed
						while (i < new_size) {
							this->_alloc.destroy(this->arr + i);
							this->_alloc.construct(this->arr + i, *first++);
							i++;
						}
						while (i < this->_size) {
							this->_alloc.destroy(this->arr + i);
							i++;
						}
					} else {
						while (i < this->_size) {
							this->_alloc.destroy(this->arr + i);
							this->_alloc.construct(this->arr + i, *first++);
							i++;
						}
						while (i < new_size) {
							this->_alloc.construct(this->arr + i, *first++);
							i++;
						}
					}
					this->_size = new_size; // Update the size with the new one
				}

			void assign(size_type n, const value_type& val) { // Fill // new contents are n elements, each initialized to a copy of val
				reserve(n);
				size_type i = 0;

				if (n < this->_size)
				{
					while (i < n)
					{
						this->_alloc.destroy(this->arr + i);
						this->_alloc.construct(this->arr + i, val);
						i++;
					}
					while (i < this->_size)
					{
						this->_alloc.destroy(this->arr + i);
						i++;
					}
				}
				else {
					while (i < this->_size)
					{
						this->_alloc.destroy(this->arr + i);
						this->_alloc.construct(this->arr + i, val);
						i++;
					}
					while (i < n)
					{
						this->_alloc.construct(this->arr + i, val);
						i++;
					}
				}
				this->_size = n;
			}

								/*  push/pop_back  */

			void push_back(const value_type& val) {		// Add element to the end of the vector
				if (this->_size + 1 > this->_capacity) { // If size > at current capacity
					size_type new_capacity = this->_capacity; // Stock the current capacity in new_capacity
					if (new_capacity == 0) // If the capacity = 0, we can t double 0 so we initialized new_capacity = 1
						new_capacity = 1;
					else
						new_capacity *= 2; // Need more capacity so reallocate the double of current capacity
					this->reallocate(new_capacity);
				}
				this->_alloc.construct(this->arr + this->_size, val);
				this->_size++; // Size increases as we add an element to the vector
			}

			void pop_back() { 	// Delete the last element in the vector
				this->erase(this->end() - 1);
			}

								/*   Insert   */
			// Vector is extended by inserting new elements before the element at the specified position
			iterator insert(iterator position, const value_type& val) { // Single element
				size_type id = position - this->begin();	// calculate the distance between the adress of begin() and the adress of position (adresse - adresse = int distance)
				if (this->_size + 1 > this->_capacity)		// Need to reallocate cause size + 1 > at current capacity
					this->reallocate(this->_size + 1);
				if (this->empty()) {	// If it's empty, just need to construct the new element at the begin() and we can t use an iterator in an empty array
					this->_alloc.construct(this->arr, val);
					this->_size++;
					return this->begin();
				}
				for (size_type i = this->_size; i > id; i--) // Offset(decaler) the element of the vector from the end to before the id to let spaces for new elements // Offset the end to the right to insert the element at position 
					this->arr[i] = this->arr[i - 1];
				this->_alloc.construct(this->arr + id, val);
				this->_size++;								 // Increased the size by + 1 for the new element
				return this->begin() + id; // return begin + position - begin = position But as we have shifted the elements the position address 
								// is not valid anymore so we use id to get our new position
			}

			void insert(iterator position, size_type n, const value_type& val) { // Fill
				if (n) {
					size_type id = position - this->begin();
					if (this->_size + n > this->_capacity) {
						if (this->_size * 2 > this->_size + n)
							this->reallocate(this->_size * 2);
						else
							this->reallocate(this->_size + n);
					}
					if (this->empty()) {
						for (size_type i = 0; i < n; i++)
							this->_alloc.construct(this->arr + i, val);
						this->_size += n;
					}
					else {
						for (size_type i = this->_size + n - 1; i >= id + n; i--) {
							if (i >= this->_size)
								this->_alloc.construct(this->arr + i, this->arr[i - n]);
							else
								this->arr[i] = this->arr[i - n];
						}
						for (size_type i = 0; i < n; i++) {
							if (id + i < this->_size) {
								this->_alloc.destroy(this->arr + id + i);
								this->_alloc.construct(this->arr + id + i, val);
							}
							else
								this->_alloc.construct(this->arr + id + i, val);
						}
						this->_size += n;
					}
				}
			}

			template<class InputIterator>
				void insert(iterator position, InputIterator first, InputIterator last,
					typename ft::enable_if<!ft::is_integral<InputIterator>::value, InputIterator>::type* = NULL) { // Range
					size_type id = position - this->begin();
					size_type new_size = 0;
					for (InputIterator it = first; it != last; it ++)
						new_size++;
					if (this->_size + new_size > this->_capacity)
						this->reallocate(this->_size * 2 + new_size);
					if (this->empty()) {
						size_type i = 0;
						for (InputIterator it = first; it != last; it++, i++)
							this->_alloc.construct(this->arr + i, *it);
						this->_size += new_size;
					}
					else {
						for (size_type i = this->_size + new_size - 1; i >= id + new_size; i--)
							if (i >= this->_size)
								this->_alloc.construct(this->arr + i, this->arr[i - new_size]);
							else
								this->arr[i] = this->arr[i - new_size];
						size_type i = 0;
						for (InputIterator it = first; it != last; it ++, i++) {
							if (id + i < this->_size) {
								this->_alloc.destroy(this->arr + id + i);
								this->_alloc.construct(this->arr + id + i, *it);
							}
							else {
								this->_alloc.construct(this->arr + id + i, *it);
							}
						}
						this->_size += new_size;
					}
				}
						/*   Erase   */

			iterator erase(iterator position) {		// Remove a single element at position
				return (this->erase(position, position + 1));
			}

			iterator erase(iterator first, iterator last) {		// Remove a range of elements from first to last
				if (first == this->end() || first == last)
					return first;

				size_type begin = 0;
				size_type first_to_last = 0;
				size_type last_to_end = 0;
				size_type i = 0;

				iterator it = this->begin();

				//count begin
				while (it != first) {
					begin++;
					it++;
				}
				it = first;
				while (it != last) {
					first_to_last++;
					it ++;
				}

				//count last to end
				it = last;
				while (it != this->end()) {
					last_to_end++;
					it++;
				}
				//Allows not to lose the first ones which would not be to remove
				while (i < last_to_end)
				{
					this->_alloc.destroy(&this->arr[begin + i]);
					this->_alloc.construct(&this->arr[begin + i], this->arr[begin + i + first_to_last]);
					i++;
				}
				// Destroy tout entre first et last
				i = 0;
				while (i < first_to_last) {
					this->_alloc.destroy(&this->arr[begin + last_to_end + i]);
					i++;
				}
				this->_size -= first_to_last;
				return first;
			}

						/*  Swap/Clear  */

			void swap(vector& x) {		// Exchange the content of the container by the content of x
				pointer tmp = this->arr;
				this->arr = x.arr;
				x.arr = tmp;
				size_type tmp_size = this->_size;
				this->_size = x._size;
				x._size = tmp_size;
				size_type tmp_capacity = this->_capacity;
				this->_capacity = x._capacity;
				x._capacity = tmp_capacity;
			}

			void clear() {				// Remove all elements from the vector(which are destroyed), leaving the container with a size of 0
				for (size_type i = 0; i < this->_size; i++)
					this->_alloc.destroy(this->arr + i);
				this->_size = 0;
			}

			// Allocator

			allocator_type get_allocator() const {
				return this->_alloc;
			}
			
		private:
			pointer			arr;
			allocator_type	_alloc;
			size_type		_size; // cause size already taken by method
			size_type		_capacity; // allocated size of the vector
			size_type		old_resize;

			// Get the error message for out_of_range exception
			std::string out_of_range_what(size_type n) const {
				std::stringstream ss;
				std::string what = "vector::_M_range_check: __n (which is ";
				ss << n;
				what.append(ss.str());
				what.append(") >= this->size() (which is ");
				ss.str("");
				ss << this->size();
				what.append(ss.str());
				what.push_back(')');
				return what;
			}

			// Allocation Manager
			void allocate(size_type n) {
				if (n > this->max_size())
					throw std::length_error("vector::reserve");
				this->arr = NULL;
				if (n > this->_capacity) {
					this->arr = this->_alloc.allocate(n);
					this->_capacity = n;
				}
			}

			void deallocate() {
				this->deallocate(this->arr, this->_capacity);
				this->_capacity = 0;
			}

			void deallocate(pointer p, size_type capacity) {
				for (size_type i = 0; i < this->_size; i++)
					this->_alloc.destroy(p + i);
				if (p)
					this->_alloc.deallocate(p, capacity);
			}

			void reallocate(size_type new_capacity) {
				pointer tmp = this->_alloc.allocate(new_capacity);
				for (size_type i = 0; i < this->_size; i++)
				{
					this->_alloc.construct(tmp + i, this->arr[i]);
					this->_alloc.destroy(&this->arr[i]);
				}
				this->_alloc.deallocate(this->arr, this->_capacity);
				this->arr = tmp;
				this->_capacity = new_capacity;
			}
	};

	// Relational Operators

template <class T, class Alloc>
	bool operator==(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs) {
		if (lhs.size() != rhs.size())
			return false;
		for (typename vector<T>::size_type i = 0; i < lhs.size(); i++)
			if (lhs[i] != rhs[i])
				return false;
		return true;
	}

template <class T, class Alloc>
	bool operator!=(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs) {
		return !(lhs == rhs);
	}

template <class T, class Alloc>
	bool operator<(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs) {
		return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
	}

template <class T, class Alloc>
	bool operator<=(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs) {
		return !(rhs < lhs);
	}

template <class T, class Alloc>
	bool operator>(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs) {
		return rhs < lhs;
	}

template <class T, class Alloc>
	bool operator>=(const vector<T, Alloc>& lhs, const vector<T, Alloc>& rhs) {
		return !(lhs < rhs);
	}

	// Swap non member
template <class T, class Alloc>
	void swap(vector<T, Alloc>& x, vector<T, Alloc>& y) {
		x.swap(y);
	}
}

#endif // VECTOR_HPP
