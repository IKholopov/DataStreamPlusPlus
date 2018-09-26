#pragma once

#include <memory>
#include <functional>

#include <DataStream.h>

template<typename DataT>
class RecurrentEmitter : public Emitter<DataT> {
public:
    RecurrentEmitter(std::function<const DataT( const DataT )> recurrentFormula,
                     const DataT& startingState):
        recurrentFormula(recurrentFormula),
        startingState(startingState),
        state(startingState)
    {
    }

    RecurrentEmitter(const RecurrentEmitter& other):
        recurrentFormula(other.recurrentFormula),
        startingState(other.startingState),
        state(other.state)
    {
    }

    virtual const DataT EmitNext() {
        const DataT oldState = state;
        state = recurrentFormula(state);
        return oldState;
    }
    virtual bool Ended() override {
        return false;
    }
    virtual void Reset() {
        state = startingState;
    }

    virtual std::unique_ptr<Emitter<DataT>> Copy() const
        { return std::make_unique<RecurrentEmitter>(recurrentFormula, startingState); }
    virtual std::unique_ptr<Emitter<DataT>> CopyState() const
        { return std::make_unique<RecurrentEmitter>(*this); }

private:
    std::function<const DataT( const DataT )> recurrentFormula;
    const DataT startingState;
    DataT state;
};

template<typename DataT>
class RecurrentContainer: public DataStream<DataT> {
public:
    RecurrentContainer(std::function<const DataT( const DataT )> recurrentFormula, const DataT& startingState):
        DataStream<DataT>(std::make_unique< const RecurrentEmitter<DataT> >(recurrentFormula, startingState).get())
    {
    }
};
