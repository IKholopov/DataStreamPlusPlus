#pragma once

#include <memory>
#include <functional>

#include <Emitter.h>

template<typename DataT>
class WhereEmitter : public Emitter<DataT> {
public:
    WhereEmitter(const Emitter<DataT>* emitter,
                   std::function<bool( const DataT )> filter):
        emitter(std::move(emitter->Copy())),
        filter(filter),
        lastValue(nullptr)
    {
        updateLast();
    }

    WhereEmitter(const WhereEmitter& emitter):
        emitter(std::move(emitter.emitter->CopyState())),
        filter(emitter.filter),
        lastValue(emitter.lastValue.get() == nullptr ? nullptr : new DataT(*emitter.lastValue))
    {}

    virtual const DataT EmitNext() {
        DataT val = *lastValue;
        updateLast();
        return val;
    }
    virtual bool Ended() { return lastValue == nullptr; }
    virtual void Reset() {
        emitter->Reset();
        updateLast();
    }

    virtual std::unique_ptr<Emitter<DataT>> Copy() const
        { return std::make_unique<WhereEmitter>(emitter.get(), filter); }
    virtual std::unique_ptr<Emitter<DataT>> CopyState() const
        { return std::make_unique<WhereEmitter>(*this); }

private:
    std::unique_ptr< Emitter<DataT> > emitter;
    std::function<bool( const DataT )> filter;
    std::unique_ptr<DataT> lastValue;

    void updateLast() {
        while(!emitter->Ended()) {
            lastValue.reset(new DataT(emitter->EmitNext()));
            if(filter(*lastValue)) {
                return;
            }
        }
        lastValue.reset(nullptr);
    }
};
