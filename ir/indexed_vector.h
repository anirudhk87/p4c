/*
Copyright 2013-present Barefoot Networks, Inc.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

#ifndef IR_INDEXED_VECTOR_H_
#define IR_INDEXED_VECTOR_H_


#include "dbprint.h"
#include "lib/enumerator.h"
#include "lib/null.h"
#include "lib/error.h"
#include "vector.h"
#include "id.h"

namespace IR {

// declaration interface: objects with names
class IDeclaration : public virtual INode {
 public:
    // name of the declared object
    virtual ID getName() const = 0;
    // User-visible name of the object, with optional replacement if no explicit @name
    virtual cstring externalName(cstring replace = cstring()) const;
    virtual ~IDeclaration() {}
};

// A Vector that also keeps an index of all IDeclaration objects.

template<class T>
class IndexedVector : public Vector<T> {
    ordered_map<cstring, const IDeclaration*> declarations;

    void insertInMap(const T* a) {
        if (!a->template is<IDeclaration>())
            return;
        auto decl = a->template to<IDeclaration>();
        auto name = decl->getName().name;
        auto previous = declarations.find(name);
        if (previous != declarations.end())
            ::error("%1%: Duplicates declaration %2%", a, previous->second);
        else
            declarations[name] = decl; }
    void removeFromMap(const T* a) {
        auto decl = a->template to<IDeclaration>();
        if (decl == nullptr)
            return;
        cstring name = decl->getName().name;
        auto it = declarations.find(name);
        if (it == declarations.end())
            BUG("%1% does not exist", a);
        declarations.erase(it); }

 public:
    using Vector<T>::begin;
    using Vector<T>::end;

    IndexedVector() = default;
    IndexedVector(const IndexedVector &) = default;
    IndexedVector(IndexedVector &&) = default;
    IndexedVector(const std::initializer_list<const T *> &a) : Vector<T>(a) {
        for (auto el : *this) insertInMap(el); }
    IndexedVector &operator=(const IndexedVector &) = default;
    IndexedVector &operator=(IndexedVector &&) = default;
    explicit IndexedVector(const T *a) {
        push_back(std::move(a)); }
    explicit IndexedVector(const vector<const T *> &a) {
        insert(typename Vector<T>::end(), a.begin(), a.end()); }
    explicit IndexedVector(const Vector<T> &a) {
        insert(typename Vector<T>::end(), a.begin(), a.end()); }
    explicit IndexedVector(JSONLoader &json);

    void clear() { IR::Vector<T>::clear(); declarations.clear(); }
    // Although this is not a const_iterator, it should NOT
    // be used to modify the vector directly.  I don't know
    // how to enforce this property, though.
    typedef typename Vector<T>::iterator iterator;

    const IDeclaration* getDeclaration(cstring name) const {
        auto it = declarations.find(name);
        if (it == declarations.end())
            return nullptr;
        return it->second; }
    template <class U>
    const U* getDeclaration(cstring name) const {
        auto it = declarations.find(name);
        if (it == declarations.end())
            return nullptr;
        return it->second->template to<U>(); }
    Util::Enumerator<const IDeclaration*>* getDeclarations() const {
        return Util::Enumerator<const IDeclaration*>::createEnumerator(
            Values(declarations).begin(), Values(declarations).end()); }
    iterator erase(iterator i) {
        removeFromMap(*i);
        return Vector<T>::erase(i); }
    template<typename ForwardIter>
    iterator insert(iterator i, ForwardIter b, ForwardIter e) {
        for (auto it = b; it != e; ++it)
            insertInMap(*it);
        return Vector<T>::insert(i, b, e); }
    iterator replace(iterator i, const T* v) {
        removeFromMap(*i);
        *i = v;
        insertInMap(v);
        return ++i; }
    iterator append(const Vector<T>& toAppend) {
        return insert(Vector<T>::end(), toAppend.begin(), toAppend.end()); }
    iterator insert(iterator i, const T* v) {
        insertInMap(v);
        return Vector<T>::insert(i, v); }
    template <class... Args> void emplace_back(Args&&... args) {
        auto el = new T(std::forward<Args>(args)...);
        insert(el); }
    void push_back(T *a) { CHECK_NULL(a); Vector<T>::push_back(a); insertInMap(a); }
    void push_back(const T *a) { CHECK_NULL(a); Vector<T>::push_back(a); insertInMap(a); }
    void pop_back() {
        if (typename Vector<T>::empty())
            BUG("pop_back from empty IndexedVector");
        auto last = typename Vector<T>::back();
        removeFromMap(last);
        typename Vector<T>::pop_back(); }
    template<class U> void push_back(U &a) { Vector<T>::push_back(a); insertInMap(a); }

    IRNODE_SUBCLASS(IndexedVector)
    IRNODE_DECLARE_APPLY_OVERLOAD(IndexedVector)
    bool operator==(const Node &a) const override { return a == *this; }
    bool operator==(const Vector<T> &a) const override { return a == *this; }
    bool operator==(const IndexedVector &a) const override {
        return Vector<T>::operator==(static_cast<const Vector<T>&>(a)); }
    cstring node_type_name() const override {
        return "IndexedVector<" + T::static_type_name() + ">"; }
    static cstring static_type_name() {
        return "IndexedVector<" + T::static_type_name() + ">"; }
    void visit_children(Visitor &v) override;
    void visit_children(Visitor &v) const override;

    void toJSON(JSONGenerator &json) const override;
    static IndexedVector<T>* fromJSON(JSONLoader &json);
    void check_valid() const {
        for (auto el : *this) {
            auto it = declarations.find(el->getName());
            assert(it != declarations.end() && it->second == el); } }
};

}  // namespace IR

#endif /* IR_INDEXED_VECTOR_H_ */
