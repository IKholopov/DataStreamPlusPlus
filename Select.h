#pragma once

#include <memory>
#include <functional>

#include <Emitter.h>

template<typename DataT, typename OutDataT>
class SelectEmitter : public Emitter<OutDataT> {
public:
    SelectEmitter(const Emitter<DataT>* emitter,
                   std::function<OutDataT( const DataT )> filter):
        emitter(std::move(emitter->Copy())),
        filter(filter)
    {}

    SelectEmitter(const SelectEmitter& emitter):
        emitter(std::move(emitter.emitter->CopyState())),
        filter(emitter.filter)
    {}

    virtual const OutDataT EmitNext() override { return filter(emitter->EmitNext()); }
    virtual bool Ended() override { return emitter->Ended(); }
    virtual void Reset() override { emitter->Reset(); }

    virtual std::unique_ptr<Emitter<OutDataT>> Copy() const override
        { return std::make_unique<SelectEmitter>(emitter.get(), filter); }
    virtual std::unique_ptr<Emitter<OutDataT>> CopyState() const override
        { return std::make_unique<SelectEmitter>(*this); }

private:
    std::unique_ptr< Emitter<DataT> > emitter;
    std::function<OutDataT( const DataT )> filter;
};
