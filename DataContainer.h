#pragma once

#include <functional>
#include <DataStream.h>


template<typename DataT, typename DataHolderT=std::vector<DataT>>
class Container : public DataStream<DataT>, public Emitter<DataT> {
public:
    Container( const DataHolderT& data) :
        DataStream<DataT>(this),
        holder(std::make_shared<DataHolderT>(data)),
        i(0)
    {}

    Container( const Container& other) :
        DataStream<DataT>(this),
        holder(other.holder),
        i(0)
    {}

    Container( const Container&& other) :
        DataStream<DataT>(this),
        holder(other.holder),
        i(0)
    {}

    virtual const DataT EmitNext() override { return (*holder)[i++]; }
    virtual bool Ended() override { return i == holder->size(); }
    virtual void Reset() override { i = 0; }

    virtual std::unique_ptr<Emitter<DataT>> Copy() const { return std::make_unique<Container>(*this); }
    virtual std::unique_ptr<Emitter<DataT>> CopyState() const {
        auto copy = std::make_unique<Container>(*this);
        copy->i = i;
        return copy;
    }

private:
    std::shared_ptr<const DataHolderT> holder;
    int i;
};
