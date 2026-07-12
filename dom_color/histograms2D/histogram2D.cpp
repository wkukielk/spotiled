#include "histogram2D.hpp"
#include <cstring>


namespace hist2D {
    struct MaxCluster {
        const MyCluster* cluster = nullptr;
        uint32_t count = 0;
        uint8_t depth = 0;
    };

    Histogram::Histogram(uint16_t exclusive_upper_bound_x, uint16_t exclusive_upper_bound_y) :
        clusters{(MyCluster(*)[sz][sz])(new MyCluster[sz * sz]{})},
        upper_bound_x{exclusive_upper_bound_x}, upper_bound_y{exclusive_upper_bound_y} {}

    Histogram::~Histogram() { delete[] clusters; }

    const MyCluster (*Histogram::get_clusters() const)[sz][sz] {
        return clusters;
    }

    Index Histogram::add(Value value) {
        auto index = Index{};
        index.x = value.x * sz / upper_bound_x;
        index.y = value.y * sz / upper_bound_y;
        (*clusters)[index.x][index.y].add(value);
        ++count;
        return index;
    }

    Index Histogram::add(Value value, uint8_t weight) {
        auto index = Index{};
        index.x = value.x * sz / upper_bound_x;
        index.y = value.y * sz / upper_bound_y;
        (*clusters)[index.x][index.y].add(value, weight);
        count += weight;
        return index;
    }

    void Histogram::reset() {
        count = 0;
        memset(clusters, 0, sz * sz * sizeof(MyCluster));
    }

    Index Histogram::get_index(const MyCluster* cluster) const {
        uint8_t index = cluster - (const MyCluster*)(clusters);
        uint8_t y_row = index / sz;
        uint8_t x_row = index - sz * y_row;
        return { .x = x_row, .y = y_row };
    }
};
