#pragma once

#include <string>
#include <iostream>
#include <stdexcept>
#include <utility>

#include "Base_List.h"

// Template implementation must be in the header file
template <class T>
class Named_List : public Base_List
{
public:
    Named_List();
    ~Named_List();
    Named_List(const Named_List &Other);
    Named_List &operator=(const Named_List &Other);
    Named_List(Named_List &&Other) noexcept;
    Named_List &operator=(Named_List &&Other) noexcept;
    void Clear() noexcept;
    void Swap_Storage(Named_List &Other) noexcept;


    /*********************************************************
     *  Functions for Adding and removing Data from the List  *
     *********************************************************/
    void Add_Node();
    void Add_Node(std::string Name);
    void Add_Node(T Data);
    void Add_Node(std::string Name, T Data);
    void operator+=(T Data); // Simplified way of using Add_Node(T Data);
    void Remove();
    void Remove(std::string Name);
    /*****************************************
     *	Pop() calls Out() and then Remove()  *
     *****************************************/
    T Pop();
    T Pop(int N);

    /*****************************************
     *   Functions for navigating the list    *
     *****************************************/

    void Next();
    void Back();
    void Go_To(int N);            // Goes to the nth Node in the list
    bool Find(std::string Name); // returns true if Name is found
    void Last();
    T *operator[](int N);
    T *operator[](std::string Name);

    /********************************************
     *	Functions for moving data in the List   *
     ********************************************/
    void Display();
    void Move_Up();
    void Move_Down();
    void Swap(int a, int b);
    void Swap(std::string a, std::string b);
    /*
        Functions for Data Output
    */

    T Out();      // Return the Value of the Current Node
    T Out(int N); // Return the Value of the Nth Node

    // Returns the address of Value in the Nth Node
    T *GetAddress(int N);
    // Returns the address of Value in Named node
    T *GetAddress(std::string Name);

    struct Node
    {
        std::string Name;
        T Value{};
        Node *Next_Node = nullptr;
        Node *Back_Node = nullptr;
    };

    void Swap_Adjacent(Node *A, Node *B);

    Node *Start;   // The first Node for circular linking
    Node *Current; // The Current Node to be used
};


template <class T>
Named_List<T>::Named_List()
{
    Base_List::Looped = true;
    Start = 0;
    Current = 0;
    Base_List::Nodes = 0;
    Base_List::Current_Node = 0;
    Base_List::Current_Name = "";
}

template <class T>
void Named_List<T>::Display()
{
    if(!Current) return;
    Go_To(1);
    for(int I = 0; I < Nodes; ++I) {
        std::cout << Current_Name << std::endl;
        Next();
    }
}

template <class T>
bool Named_List<T>::Find(std::string Name)
{
    if (Nodes < 1)
        return false;
    bool Done = false;
    if (Nodes < 2 or Current->Name == Name)
    {
        Done = true;
    }
    else
        Go_To(1);

    while (Done == false)
    {
        if (Current->Name == Name)
            Done = true;
        else if (Current_Node == Nodes)
            Done = true;
        else
            Next();
    }
    Current_Name = Current->Name;
    if (Current->Name == Name)
        return true;
    else
        return false;
}

template <class T>
void Named_List<T>::Last()
{
    Go_To(Nodes);
}

template <class T>
void Named_List<T>::Swap(int a, int b)
{
    if(!Current) return;
    Go_To(a);
    Node *A = Current;
    Go_To(b);
    Node *B = Current;
    using std::swap;
    swap(A->Name, B->Name);
    swap(A->Value, B->Value);
    Go_To(a);
}

template <class T>
void Named_List<T>::Move_Up()
{
    if(Nodes < 2 || (!Looped && Current == Start)) return;
    int Index = Current_Node == 1 ? Nodes : Current_Node - 1;
    Swap_Adjacent(Current->Back_Node, Current);
    Current_Node = Index;
    Current_Name = Current->Name;
}

template <class T>
void Named_List<T>::Move_Down()
{
    if(Nodes < 2 || (!Looped && Current == Start->Back_Node)) return;
    int Index = Current_Node == Nodes ? 1 : Current_Node + 1;
    Swap_Adjacent(Current, Current->Next_Node);
    Current_Node = Index;
    Current_Name = Current->Name;
}

template <class T>
Named_List<T>::~Named_List()
{
    Clear();
}

template <class T>
void Named_List<T>::Add_Node()
{
    if (Nodes == 0) // if this is the first Node in the Named_List
    {
        Start = new Node;
        Start->Back_Node = Start;
        Start->Next_Node = Start;
        Current = Start;
        Nodes = 1;
        Current_Node = 1;
        Current_Name = Current->Name;
    }
    else if (Nodes >= 1)
    {
        // Ensure new Node is added to the end
        Current = Start->Back_Node;
        Current_Node = Nodes;

        // Make the Current Node point to a new Node
        Current->Next_Node = new Node;

        // Make the New node point back
        Current->Next_Node->Back_Node = Current;

        // Make the new Node the Current Node
        Current = Current->Next_Node;

        // Link the new node to the Start Node
        Current->Next_Node = Start;
        // Link the Start node back to the new Node
        Start->Back_Node = Current;
        Nodes++;
        Current_Node++;
        Current_Name = Current->Name;
    }
    else
    {
        std::cout << "Error negative number of nodes in Named List" << std::endl;
    }
}

template <class T>
void Named_List<T>::Add_Node(T Data)
{
    Add_Node();
    Current->Value = std::move(Data);
    Current_Name = Current->Name;
}

template <class T>
void Named_List<T>::Add_Node(std::string Name, T Data)
{
    Add_Node(std::move(Data));
    Current->Name = Name;
    Current_Name = Current->Name;
}

template <class T>
void Named_List<T>::Add_Node(std::string Name)
{
    Add_Node();
    Current->Name = Name;
    Current_Name = Current->Name;
}

template <class T>
void Named_List<T>::Next()
{
    if(!Current) return;
    if (Nodes > 1) // Make sure there is a next Node
    {
        if (Looped)
        {
            // Increment the index
            Current_Node++;
            // Move to the next Node
            Current = Current->Next_Node;
            // if you moved to the Start Node make the index 1
            if (Current == Start)
                Current_Node = 1;
        }
        else
        {
            if (Current != Start->Back_Node)
            {
                // Increment the index
                Current_Node++;
                // Move to the next Node
                Current = Current->Next_Node;
            }
        }
    }
    Current_Name = Current->Name;
}

template <class T>
void Named_List<T>::Back()
{
    if(!Current) return;
    if (Nodes > 1) // Make Sure there is a Node to go back to
    {
        if (Looped)
        {
            // Decremnet the index
            Current_Node--;
            // Move back one
            Current = Current->Back_Node;
            // if you move past the first Node set the index to the end
            if (Current == Start->Back_Node)
                Current_Node = Nodes;
        }
        else
        {
            if (Current != Start)
            {
                // Decrement the index
                Current_Node--;
                // Move back one
                Current = Current->Back_Node;
            }
        }
    }
    Current_Name = Current->Name;
}

template <class T>
void Named_List<T>::operator+=(T Data)
{
    Add_Node(std::move(Data));
    Current_Name = Current->Name;
}

template <class T>
T Named_List<T>::Out()
{
    if(!Current) throw std::out_of_range("Cannot read an empty Named_List");
    return Current->Value;
}

template <class T>
T Named_List<T>::Out(int n)
{
    Go_To(n);
    return Out();
}
template <class T>
T Named_List<T>::Pop(int n)
{
    Go_To(n);
    return Pop();
}

template <class T>
T Named_List<T>::Pop()
{
    T ReturnValue = Out();
    Remove();
    return ReturnValue;
}

template <class T>
void Named_List<T>::Remove()
{
    if(!Current || Nodes == 0) return;
    Node *Removed = Current;
    if(Nodes == 1) {
        Start = Current = nullptr;
        Nodes = Current_Node = 0;
        Current_Name.clear();
    } else {
        Removed->Next_Node->Back_Node = Removed->Back_Node;
        Removed->Back_Node->Next_Node = Removed->Next_Node;
        if(Removed == Start) Start = Removed->Next_Node;
        Current = Removed->Next_Node;
        --Nodes;
        if(Current == Start) Current_Node = 1;
        Current_Name = Current->Name;
    }
    delete Removed;
}

template <class T>
void Named_List<T>::Remove(std::string Name)
{
    if (Find(Name))
    {
        Remove();
    }
}

template <class T>
void Named_List<T>::Go_To(int N)
{
    // Speacial Cases

    // If N is to high goto the last Node
    if ((N >= Nodes) && (Nodes > 0)) //|| (N < 0 && Nodes > 0))
    {
        Current = Start->Back_Node;
        Current_Node = Nodes;
    }

    // If N is 0 or negative goto Start
    if (N <= 1 && Nodes > 0)
    {
        Current = Start;
        Current_Node = 1;
    }

    // Normal cases
    if (N < Current_Node && N > 1)
    {
        for (; N < Current_Node; Back())
            ;
    }
    if (N > Current_Node && N < Nodes)
    {
        for (; N > Current_Node; Next())
            ;
    }

    if (Nodes > 0)
        Current_Name = Current->Name;
}

template <class T>
T *Named_List<T>::GetAddress(int N)
{
    Go_To(N);
    return Current ? &Current->Value : nullptr;
}

template <class T>
T *Named_List<T>::GetAddress(std::string Name)
{
    return Find(Name) ? &Current->Value : nullptr;
}

template <class T>
T *Named_List<T>::operator[](int N)
{
    Go_To(N);
    return Current ? &Current->Value : nullptr;
}

template <class T>
T *Named_List<T>::operator[](std::string Name)
{
    return Find(Name) ? &Current->Value : nullptr;
}


template<class T>
void Named_List<T>::Swap_Adjacent(Node *A, Node *B)
{
    if(Start == A) Start = B;
    else if(Start == B) Start = A;
    if(Nodes == 2) return;
    Node *Before = A->Back_Node, *After = B->Next_Node;
    Before->Next_Node = B; B->Back_Node = Before;
    B->Next_Node = A; A->Back_Node = B;
    A->Next_Node = After; After->Back_Node = A;
}

template<class T>
void Named_List<T>::Clear() noexcept
{
    // Detach before destruction; values may run plugin cleanup code.
    Node *First = Start;
    int Count = Nodes;
    Start = Current = nullptr;
    Nodes = Current_Node = 0;
    Current_Name.clear();
    for(int I = 0; I < Count && First != nullptr; ++I) {
        Node *Next = First->Next_Node;
        delete First;
        First = Next;
    }
}

template<class T>
void Named_List<T>::Swap_Storage(Named_List &Other) noexcept
{
    using std::swap;
    swap(Start, Other.Start); swap(Current, Other.Current);
    swap(Nodes, Other.Nodes); swap(Current_Node, Other.Current_Node);
    swap(Current_Name, Other.Current_Name); swap(Looped, Other.Looped);
}

template<class T>
Named_List<T>::Named_List(const Named_List &Other) : Named_List()
{
    try {
        const Node *Cursor = Other.Start;
        for(int I = 0; I < Other.Nodes; ++I) {
            Add_Node(Cursor->Name, Cursor->Value);
            Cursor = Cursor->Next_Node;
        }
        Looped = Other.Looped;
        Go_To(Other.Current_Node);
    } catch(...) { Clear(); throw; }
}

template<class T>
Named_List<T> &Named_List<T>::operator=(const Named_List &Other)
{
    if(this != &Other) { Named_List Copy(Other); Swap_Storage(Copy); }
    return *this;
}

template<class T>
Named_List<T>::Named_List(Named_List &&Other) noexcept : Named_List()
{
    Swap_Storage(Other);
}

template<class T>
Named_List<T> &Named_List<T>::operator=(Named_List &&Other) noexcept
{
    if(this != &Other) { Clear(); Swap_Storage(Other); }
    return *this;
}
