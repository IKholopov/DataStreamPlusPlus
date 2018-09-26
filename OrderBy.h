#pragma once

#include <memory>
#include <functional>
#include <unordered_set>
#include <Emitter.h>

template<typename DataT>
class DataStream;

template<typename DataT, typename DataHolderT>
class Container;

template<typename DataT, typename KeyT>
class OrderByEmitter : public Emitter<DataT> {
public:
    OrderByEmitter(const Emitter<DataT>* emitter,
                   std::function<const KeyT( const DataT )> filter):
        dataContainer(nullptr),
        emitter(std::move(emitter->Copy())),
        filter(filter),
        initialized(false)
    {
    }

    OrderByEmitter(const OrderByEmitter& other):
        emitter(std::move(other.emitter->CopyState())),
        filter(other.filter),
        initialized(other.initialized)
    {
        auto otherContainer = other.dataContainer->CopyState();
        dataContainer = std::move(castToContainer(otherContainer));
    }

    virtual const DataT EmitNext() override {
        if(!initialized) {
            initialize();
        }
        return dataContainer->EmitNext();
    }

    virtual bool Ended() override {
        if(!initialized) {
            initialize();
        }
        return dataContainer->Ended();
    }
    virtual void Reset() override {
        emitter->Reset();
        initialize();
    }

    virtual std::unique_ptr<Emitter<DataT>> Copy() const
        { return std::make_unique<OrderByEmitter>(emitter.get(), filter); }
    virtual std::unique_ptr<Emitter<DataT>> CopyState() const
        { return std::make_unique<OrderByEmitter>(*this); }

private:
    std::unique_ptr<Container< DataT, std::vector<DataT> > > dataContainer;
    std::unique_ptr< Emitter<DataT> > emitter;
    std::function<const KeyT( const DataT )> filter;
    bool initialized;

    void initialize() {
        initialized = true;
        auto v = DataStream<DataT>(emitter.get()).ToVector();

        std::sort(v.begin(), v.end(), [f=filter](DataT a, DataT b){
            return f(a) < f(b);
        });

        dataContainer.reset(new Container< DataT, std::vector<DataT> >(v));
    }

    std::unique_ptr<Container< DataT, std::vector<DataT> > > castToContainer(std::unique_ptr<Emitter<DataT> >& ptr) {
        return std::unique_ptr<Container< DataT, std::vector<DataT> > >(static_cast<Container< DataT, std::vector<DataT> >* >(ptr.release()));
    }
};
