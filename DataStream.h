#pragma once

#include <type_traits>

#include <Select.h>
#include <Flatten.h>
#include <Where.h>
#include <Take.h>
#include <GroupBy.h>
#include <OrderBy.h>

template<typename DataT>
class DataStream {
public:

    using iterator = EmitterIterator<DataT>;

    explicit DataStream(Emitter<DataT>* emitter):
        externalEmitter(emitter)
    {}
    DataStream(const Emitter<DataT>* emitter):
        emitter(std::move(emitter->Copy())),
        externalEmitter(nullptr)
    {}
    DataStream(const DataStream& other):
        emitter(other.emitter.get() == nullptr ? nullptr : std::move(other.emitter->Copy())),
        externalEmitter(other.externalEmitter)
    {}

    DataStream(DataStream&& other):
        emitter(std::move(other.emitter)),
        externalEmitter(other.externalEmitter)
    {
        assert(other.externalEmitter == nullptr);
    }

    template<typename OutDataT>
    DataStream<OutDataT> Select(std::function<OutDataT( const DataT )> filter) const {
        return std::make_unique<SelectEmitter<DataT, OutDataT>>(GetEmitter(), filter).get();
    }

    template<typename InDataT, typename OutDataT, typename std::enable_if<std::is_same< DataT, DataStream<InDataT> >::value, int>::type = 0>
    DataStream<OutDataT> Flatten(std::function<OutDataT( const InDataT )> filter) const {
        return std::make_unique<FlattenEmitter<InDataT, OutDataT>>(GetEmitter(), filter).get();
    }

    DataStream<DataT> Where(std::function<bool( const DataT )> filter) const {
        return std::make_unique<WhereEmitter<DataT>>(GetEmitter(), filter).get();
    }

    DataStream<DataT> Take(int toTake) const {
        return std::make_unique<TakeEmitter<DataT>>(GetEmitter(), toTake).get();
    }

    template<typename KeyT>
    DataStream<std::pair<KeyT, DataStream<DataT> > > GroupBy(std::function<const KeyT( const DataT )> filter) {
        return std::make_unique<GroupByEmitter<DataT, KeyT>>(GetEmitter(), filter).get();
    }

    template<typename KeyT>
    DataStream<DataT> OrderBy(std::function<const KeyT( const DataT )> filter) {
        return std::make_unique<OrderByEmitter<DataT, KeyT>>(GetEmitter(), filter).get();
    }

    std::vector<DataT> ToVector() {
        std::vector<DataT> result;

        EmitterGuard<DataT> guard(*GetEmitter());

        while(!GetEmitter()->Ended()) {
            result.push_back(GetEmitter()->EmitNext());
        }

        return result;
    }

    iterator begin() const { return iterator(GetEmitter()); }
    iterator end() const { return iterator(std::make_unique< EmptyEmitter<DataT> >().get()); }

    DataStream<DataT>& operator=(const DataStream<DataT>& other) {
        if(this != &other) {
            if(other.emitter.get() == nullptr) {
                emitter.reset(nullptr);
            } else {
                emitter = std::move(other.emitter->Copy());
            }
            externalEmitter = other.externalEmitter;
        }
        return *this;
    }

protected:
    Emitter<DataT>* GetEmitter() const {
        return emitter.get() == nullptr ? externalEmitter : emitter.get();
    }

private:
    std::unique_ptr< Emitter<DataT> > emitter;
    Emitter<DataT>* externalEmitter;
};
