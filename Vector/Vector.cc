// Implementation of the templated Vector class
// ECE4893/8893 lab 3
// YOUR NAME HERE

#include <iostream> // debugging
#include "Vector.h"
#include "String.h"

// Your implementation here
// Fill in all the necessary functions below
using namespace std;
// Default constructor

template <typename T>
Vector<T>::Vector()
{
   size=0;
   list=0;
   position=0;
}
template <typename T>
T* Vector<T>::New(size_t size1)
{
	//T* address =new T[size1];//(T*) malloc(sizeof(T)*size1);
        //new (address) T();
       T* address = (T*)malloc(sizeof(T)*size1);
       position=0;

return address;
}
template <typename T>
void Vector<T>::Delete(T* list,size_t size)
{
	for(int i=0;i<position;i++)
	{
		(list+i)->~T();
		//free(list+i);
	}
   free(list);
  // delete [] list;
}
// Copy constructor
template <typename T>
Vector<T>::Vector(const Vector& rhs)
{
    size=rhs.size;
    list = (T*)malloc(sizeof(T)*size);
    position= rhs.position;
    for(int i=0;i<position;i++)
    {
	//cout<<rhs.list[i]<<endl;
        new (list+i) T(rhs.list[i]);
    }
}

// Assignment operator
template <typename T>
Vector<T>& Vector<T>::operator=(const Vector& rhs)
{
    if(this==&rhs)
    {
        return rhs;
    }
    else
    {
     //   delete(size);
      //  size=rhs.size;
        list= (String*)malloc(sizeof(T)*size);
        for (int i; i<position; i++) {
            new (list+i) T(rhs.list[i]);
            
        }
        return *this;
    }
}

#ifdef GRAD_STUDENT
// Other constructors
template <typename T>
Vector<T>::Vector(size_t nReserved)
{ // Initialize with reserved memory
    size = nReserved;
    list = New(nReserved);
}

template <typename T>
Vector<T>::Vector(size_t n, const T& t)
{ // Initialize with "n" copies of "t"
    //cout<<"constructor\n"<<n<<endl;
    size=n;
    list = (T*) malloc(sizeof(T)*size);
    for (int i=0; i<size; i++) {
        new (list+i) T(t);
}
   position=size;
    
}
#endif

// Destructor
template <typename T>
Vector<T>::~Vector()
{
   // cout<<"calling descructor\n";
    if(size!=0)
    {
     Delete(list,position);
     //delete list;
    }
    size=0;
    position=0;
}
template <typename T>
void Vector<T>::Reserve(size_t n)
{
        if(n<size)
	{
		return;
	}

  T * newlist = (T*)malloc(sizeof(T)*n);
                for (unsigned int i = 0; i < position; i++)
                       new (newlist+i)T(list[i]);
                                Delete(list,size);
				size=n;
                                    list=newlist;
}
// Add and access front and back
template <typename T>
void Vector<T>::Push_Back(const T& rhs)
{
	if(position<size)
	{
		new(list+position) T(rhs);
		position++;
	}
      else
	{
      Reserve(size+1);
      new(list+size-1) T(rhs);
	position=size;
	}
}
template <typename T>
void Vector<T>::Push_Front(const T& rhs)
{
    if(position<size)
	{
		 for (unsigned int i = position; i > 0; i--)
			{
				(list+i)->~T();
                       		new (list+i)T(list[i-1]);
			}
		new (list)T(rhs);
		position++;
	}
	else
	{
    T * newlist = (T*)malloc(sizeof(T)*++size);
                for (unsigned int i = size-1; i > 0; i--)
                       new (newlist+i)T(list[i-1]);
                                Delete(list,size-1);
                           new (newlist)T(rhs);
                                    list=newlist;
	position=size;
	}

}

template <typename T>
void Vector<T>::Pop_Back()
{ // Remove last element
    if(size==0||position==0)
   	return;
    //delete &list[size-1];
    (list+position-1)->~T();
    position--;

}

template <typename T>
void Vector<T>::Pop_Front()
{ // Remove first element
    if(size==0||position==0)
        return;
    for(int i=0;i<position-1;i++)
	{
	(list+i)->~T();
	new(list+i) T(list[i+1]);
	}
    Pop_Back();
}

// Element Access
template <typename T>
T& Vector<T>::Front() const
{
    if(size!=0&&position!=0)
    return list[0];
}

// Element Access
template <typename T>
T& Vector<T>::Back() const
{
    if(size!=0&&position!=0)
    return list[position-1];
}

template <typename T>
T& Vector<T>::operator[](size_t i) const
{
    if(i<0||i>=size)
    {
        cout<<"error: acess out of memory";
        exit(1);
    }
    return list[i];
}

template <typename T>
size_t Vector<T>::Size() const
{
    return position;
}

template <typename T>
bool Vector<T>::Empty() const
{
    if(size==0||position==0)
        return true;
    else return false;
}

// Implement clear
template <typename T>
void Vector<T>::Clear()
{
    if(size==0)
        return;
     Delete(list,position);
     size=0;
     position=0;
}

// Iterator access functions
template <typename T>
VectorIterator<T> Vector<T>::Begin() const
{
    if(size!=0&&position!=0)
	{
       VectorIterator<T>  v(&list[0]);
       return v;
	}
    else
        return NULL;
}

template <typename T>
VectorIterator<T> Vector<T>::End() const
{
    if(size!=0&&position!=0)
     {
	VectorIterator<T> v(&list[position]);
        return v;
     }
    else return NULL;
}

#ifdef GRAD_STUDENT
// Erase and insert
template <typename T>
void Vector<T>::Erase(const VectorIterator<T>& it)
{
    int count=-1;
    for(int i=0;i<position;i++)
    {
        if((list+i) == it.current)
        {
            count=i;
            break;
        }
    }
    if(count==-1)
        return;
    else
    {
        if(count!=size-1)
        for(int i =count;i<position-1;i++)
        {
	    (list+i)->~T();
            new(list+i) T(list[i+1]);
        }
	(list+position-1)->~T();
	position--;
        
    }
}

template <typename T>
void Vector<T>::Insert(const T& rhs, const VectorIterator<T>& it)
{

    int count=-1;
    for(int i=0;i<size;i++)
    {
        if((list+i) == it.current)
        {
            count=i;
            break;
        }
        
    }
    if(count==-1)
    {
        
    }
    else
    {

        if(position<size)
	{
		new (list+position) T(list[position-1]);	
        	for(int i=position-1;i>count;i--)
        	{
		    (list+i)->~T();
		
        	    new (list+i) T(list[i-1]);
        	}
                (list+count)->~T();
        	new (list+count) T(rhs);
		position++;
	}
	else
	{
		T * newlist = (T*) malloc(sizeof(T)*(size+1));
		for(int i=0;i<count;i++)
                {

                    new (newlist+i) T(list[i]);
                }
		new (newlist+count) T(rhs);
		for(int i=count+1;i<position+1;i++)
                {

                    new (newlist+i) T(list[i-1]);
                }
		Delete(list,position);
		list=newlist;
		size=size++;
		position=size;
		
	}
  }      
    
}
#endif

// Implement the iterators

// Constructors
template <typename T>
VectorIterator<T>::VectorIterator()
{
    current =NULL;
}






template <typename T>
VectorIterator<T>::VectorIterator(T* c)
{
   // current=new T();
   current = c;
}

// Copy constructor
template <typename T>
VectorIterator<T>::VectorIterator(const VectorIterator<T>& rhs)
{
    current=rhs.current;
}

// Iterator defeferencing operator
template <typename T>
T& VectorIterator<T>::operator*() const
{
    return *current;
}

// Prefix increment
template <typename T>
VectorIterator<T>  VectorIterator<T>::operator++()
{
    return ++current;
    
}

// Postfix increment
template <typename T>
VectorIterator<T> VectorIterator<T>::operator++(int)
{
    return current++;
}
// Comparison operators
template <typename T>
bool VectorIterator<T>::operator !=(const VectorIterator<T>& rhs) const
{
    return (current!=rhs.current);
}

template <typename T>
bool VectorIterator<T>::operator ==(const VectorIterator<T>& rhs) const
{
    return (current==rhs.current);
}




