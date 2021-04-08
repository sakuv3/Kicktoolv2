<template>
    <v-img src="@/assets/history.jpg" alt=""
           gradient="to bottom, rgba(200,10,50,.1), rgba(250,100,10, 0.4)"
           width="100%"
           height="100%">

        <v-card max-width="700"
                class="mx-auto my-1"
                color="rgb(0, 0, 0, 0)">
            <v-virtual-scroll :items="history"
                              height="900"
                              item-height="75">
                <template v-slot:default="{ item }">
                    <v-card color="rgb(10, 10, 10, 0.75)"
                            class=" mx-auto my-auto align-center"
                            width="700px"
                            max-height="90px"
                            hover
                            shaped>
                        <v-card-title>
                            {{item.name}}
                            <v-chip class="ma-0" v-if="item.host" color="green" text-color="white">
                                Host
                            </v-chip>
                            <v-spacer></v-spacer>
                            <a class="mx-3">{{item.ip}}</a>
                        </v-card-title>
                    </v-card>


                    <v-divider></v-divider>
                </template>
            </v-virtual-scroll>
        </v-card>
    </v-img>
</template>

<script>
    import axios from 'axios'
    export default {
        name: 'History',
        components: {

        },

        data() {
            return {
                history: [],
                apiURL: 'http://localhost:8000/api/history/',
            }
        },
        mounted() {
            axios.get(this.apiURL).then(response => {
                this.history = response.data;
            });
        },
        methods: {

        }
    }
</script>