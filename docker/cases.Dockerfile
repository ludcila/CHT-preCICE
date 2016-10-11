FROM all

USER \
    root

# Dependencies of the auto-config utilities
RUN \
    apt-get update \
    && apt-get install -y python-yaml \
    && apt-get install -y python-lxml \
    && pip install networkx==1.11 --allow-unverified networkx

USER \
    precice

WORKDIR \
    /home/precice

RUN \
    touch modify.sh \
    && chmod +x modify.sh \
    && echo "cp -r heat-exchanger-aster ~" >> modify.sh \
    && echo "cd ~/heat-exchanger-aster" >> modify.sh \
    && echo "sed -i 's@/home/lcheung/Thesis/precice-cases/heat-exchanger-aster/solid/@@g' solid/solid.export" >> modify.sh \
    && echo "sed -i 's@/home/lcheung/Thesis/precice-cases/heat-exchanger-aster@..@g' solid/solid.export" >> modify.sh \
    && echo "sed -i 's@/home/lcheung/Thesis/CHT-preCICE/solvers/Code_Aster@"$ASTER_ADAPTER_ROOT"@g' solid/solid.export" >> modify.sh


COPY \
    /utilities/ /home/precice/CHT-preCICE/utilities/
