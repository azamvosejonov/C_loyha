    /* TODO: *t (1970-01-01 dan beri soniyalar) -> yil, oy, kun, soat, daqiqa, soniya,
     * hafta kuni va yil kuni. Kabisa yillari: 4 ga bo'linadi, 100 ga emas, lekin 400 ga - ha. */
    (void)t;
    *out = (struct tm){ 0 };
    return out;
