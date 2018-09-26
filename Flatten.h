#pragma once

#include <memory>
#include <functional>

#include <Emitter.h>

template<typename DataT>
class DataStream;

template<typename DataT,  typename OutDataT>
class FlattenEmitter : public Emitter<OutDataT> {
public:
    FlattenEmitter(const Emitter<DataStream<DataT>>* emitter,
                   std::function<OutDataT( const DataT )> filter):
        emitter(std::move(emitter->Copy())),
        filter(filter),
        iterator(this->emitter->Ended() ? std::make_unique< EmptyEmitter<DataT> >().get() : this->emitter->EmitNext().begin())
    {}

    FlattenEmitter(const FlattenEmitter& other):
        emitter(std::move(emitter->CopyState())),
        filter(filter),
        iterator(other.iterator)
    {}

    virtual const OutDataT EmitNext() {
        if(iterator == EmptyEmitter< DataT>{}.end()) {
            iterator = emitter->EmitNext().begin();
        }
        return filter(*iterator++);
    }
    virtual bool Ended() override { return emitter->Ended() && iterator == EmptyEmitter<DataT>{}.end(); }
    virtual void Reset() {
        emitter->Reset();
        iterator = emitter->EmitNext().begin();
    }

    virtual std::unique_ptr<Emitter<OutDataT>> Copy() const
        { return std::make_unique<FlattenEmitter>(emitter.get(), filter); }
    virtual std::unique_ptr<Emitter<OutDataT>> CopyState() const
        { return std::make_unique<FlattenEmitter>(*this); }

private:
    std::unique_ptr< Emitter< DataStream<DataT> > > emitter;
    std::function<OutDataT( const DataT )> filter;
    EmitterIterator< DataT > iterator;
};
