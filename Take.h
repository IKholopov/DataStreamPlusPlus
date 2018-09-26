#pragma once

#include <memory>
#include <functional>

#include <Emitter.h>

template<typename DataT>
class TakeEmitter : public Emitter<DataT> {
public:
    TakeEmitter(const Emitter<DataT>* emitter,
                   int toTake):
        emitter(std::move(emitter->Copy())),
        toTake(toTake),
        taken(0)
    {
        assert(toTake >= 0);
    }

    TakeEmitter(const TakeEmitter& emitter):
        emitter(std::move(emitter.emitter->CopyState())),
        toTake(toTake),
        taken(0)
    {}

    virtual const DataT EmitNext() {
        taken++;
        return emitter->EmitNext();
    }
    virtual bool Ended() override {
        return emitter->Ended() || taken >= toTake;
    }
    virtual void Reset() {
        emitter->Reset();
        taken = 0;
    }

    virtual std::unique_ptr<Emitter<DataT>> Copy() const
        { return std::make_unique<TakeEmitter>(emitter.get(), toTake); }
    virtual std::unique_ptr<Emitter<DataT>> CopyState() const
        { return std::make_unique<TakeEmitter>(*this); }

private:
    std::unique_ptr< Emitter<DataT> > emitter;
    int toTake;
    int taken;
};
